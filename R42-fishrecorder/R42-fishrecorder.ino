#include <Wire.h>
#include <ControlPCM186x.h>
#include <InputTDM.h>
#include <SDWriter.h>
#include <RTClock.h>
#include <Blink.h>
#include <Configurator.h>
#include <Settings.h>
#include <InputTDMSettings.h>

// Default settings: ----------------------------------------------------------
// (may be overwritten by config file logger.cfg)
#define NCHANNELS     2        // number of channels (even, from 2 to 16)
#define PREGAIN       10.0     // gain factor of preamplifier
#define SAMPLING_RATE 48000    // samples per second and channel in Hertz
#define GAIN          30.0     // dB

#define PATH          "recordings"   // folder where to store the recordings
#define FILENAME      "rec1-SDATETIME.wav"  // may include DATE, SDATE, TIME, STIME, DATETIME, SDATETIME, ANUM, NUM
#define FILE_SAVE_TIME 5*60    // seconds
#define INITIAL_DELAY  10.0    // seconds

// ----------------------------------------------------------------------------

#define LED_PIN       27

#define SOFTWARE      "R4.2 FishRecorder v1.0"

EXT_DATA_BUFFER(AIBuffer, NAIBuffer, 16*512*256)
InputTDM aidata(AIBuffer, NAIBuffer);
ControlPCM186x pcm1(Wire1, PCM186x_I2C_ADDR2, InputTDM::TDM2);
ControlPCM186x pcm2(Wire1, PCM186x_I2C_ADDR1, InputTDM::TDM2);

SDCard sdcard;
SDWriter file(sdcard, aidata);

Configurator config;
Settings settings(PATH, FILENAME, FILE_SAVE_TIME, 0.0,
                  0.0, INITIAL_DELAY);
InputTDMSettings aisettings(&aidata, SAMPLING_RATE, NCHANNELS, GAIN);                  
RTClock rtclock;
Blink blink(LED_PIN, true, LED_BUILTIN, false);


int restarts = 0;
String prevname; // previous file name


void setupStorage(char *gainstr) {
  prevname = "";
  restarts = 0;
  blink.setTiming(5000, 100, 1200);
  if (file.sdcard()->dataDir(settings.Path))
    Serial.printf("Save recorded data in folder \"%s\".\n\n", settings.Path);
  file.setWriteInterval(2*aidata.DMABufferTime());
  file.setMaxFileTime(settings.FileTime);
  file.header().setSoftware(SOFTWARE);
  if (gainstr != 0)
    file.header().setGain(gainstr);
  file.start();
}


void openNextFile() {
  blink.setRandom();
  blink.blinkMultiple(5, 0, 200, 200);
  time_t t = now();
  String fname = rtclock.makeStr(settings.FileName, t, true);
  if (fname != prevname) {
    file.sdcard()->resetFileCounter();
    prevname = fname;
  }
  fname = file.sdcard()->incrementFileName(fname);
  if (fname.length() == 0) {
    blink.clear();
    Serial.println("WARNING: failed to increment file name.");
    Serial.println("SD card probably not inserted.");
    Serial.println();
    aidata.stop();
    while (1) { yield(); };
    return;
  }
  char dts[20];
  rtclock.dateTime(dts, t);
  if (! file.openWave(fname.c_str(), -1, dts)) {
    blink.clear();
    Serial.println();
    Serial.println("WARNING: failed to open file on SD card.");
    Serial.println("SD card probably not inserted or full -> halt");
    aidata.stop();
    while (1) { yield(); };
    return;
  }
  ssize_t samples = file.write();
  if (samples == -4) {   // overrun
    file.start(aidata.nbuffer()/2);   // skip half a buffer
    file.write();                     // write all available data
    // report overrun:
    char mfs[100];
    sprintf(mfs, "%s-error0-overrun.msg", file.baseName().c_str());
    Serial.println(mfs);
    File mf = sdcard.openWrite(mfs);
    mf.close();
  }
  Serial.println(file.name());
}


void storeData() {
  if (!file.pending())
    return;
  ssize_t samples = file.write();
  if (samples < 0) {
    blink.clear();
    Serial.println();
    Serial.println("ERROR in writing data to file:");
    char errorstr[20];
    switch (samples) {
      case -1:
        Serial.println("  File not open.");
        break;
      case -2:
        Serial.println("  File already full.");
        break;
      case -3:
        aidata.stop();
        Serial.println("  No data available, data acquisition probably not running.");
        Serial.printf("  dmabuffertime = %.2fms, writetime = %.2fms\n", 1000.0*aidata.DMABufferTime(), 1000.0*file.writeTime());
        strcpy(errorstr, "nodata");
        delay(20);
        break;
      case -4:
        Serial.println("  Buffer overrun.");
        strcpy(errorstr, "overrun");
        break;
      case -5:
        Serial.println("  Nothing written into the file.");
        Serial.println("  SD card probably full -> halt");
        aidata.stop();
        while (1) {};
        break;
    }
    if (samples <= -3) {
      file.closeWave();
      char mfs[100];
      sprintf(mfs, "%s-error%d-%s.msg", file.baseName().c_str(), restarts+1, errorstr);
      Serial.println(mfs);
      File mf = sdcard.openWrite(mfs);
      mf.close();
      Serial.println();
    }
  }
  if (file.endWrite() || samples < 0) {
    file.close();  // file size was set by openWave()
#ifdef SINGLE_FILE_MTP
    aidata.stop();
    delay(50);
    Serial.println();
    Serial.println("MTP file transfer.");
    Serial.flush();
    blink.setTriple();
    MTP.begin();
    MTP.addFilesystem(sdcard, "logger");
    while (true) {
      MTP.loop();
      blink.update();
      yield();
    }
#endif      
    if (samples < 0) {
      restarts++;
      if (restarts >= 5) {
        Serial.println("ERROR: Too many file errors -> halt.");
        aidata.stop();
        while (1) { yield(); };
      }
      if (!aidata.running())
        aidata.start();
      file.start();
    }
    openNextFile();
  }
}


bool setupPCM(InputTDM &tdm, ControlPCM186x &pcm) {
  pcm.begin();
  bool r = pcm.setMicBias(false, true);
  if (!r) {
    Serial.println("not available");
    return false;
  }
  pcm.setRate(tdm, aisettings.rate());
  pcm.setupTDM(tdm, ControlPCM186x::CH2L, ControlPCM186x::CH3R,
               false, ControlPCM186x::INVERTED);
  Serial.println("configured for 2 channels");
  pcm.setSmoothGainChange(false);
  pcm.setFilters(ControlPCM186x::FIR, false);
  return true;
}


// -----------------------------------------------------------------------------

void setup() {
  blink.switchOn();
  Serial.begin(9600);
  while (!Serial && millis() < 2000) {};
  Serial.println("\n=======================================================================\n");
  rtclock.check();
  sdcard.begin();
  rtclock.setFromFile(sdcard);
  rtclock.report();
  config.setConfigFile("logger.cfg");
  config.configure(sdcard);
  //aidata.setSwapLR();
  Wire1.begin();
  Serial.printf("Setup PCM186x 1 on TDM 2: ");
  setupPCM(aidata, pcm1);
  pcm1.setGain(ControlPCM186x::ADC1L, aisettings.gain());  // signal
  pcm1.setGain(ControlPCM186x::ADC1R, 0.0);                // LED
  pcm2.setupTDM(ControlPCM186x::CH2L, ControlPCM186x::CH2R,true);
  pcm2.powerdown();
  Serial.printf("Setup PCM186x 2 on TDM 2: powered down\n");
  Serial.println();
  aidata.begin();
  aidata.check();
  aidata.start();
  aidata.report();
  blink.switchOff();
  if (settings.InitialDelay >= 2.0) {
    delay(1000);
    blink.setDouble();
    blink.delay(uint32_t(1000.0*settings.InitialDelay)-1000);
  }
  else
    delay(uint32_t(1000.0*settings.InitialDelay));
  Serial.printf("ADC1R g=%.1fdB\n", pcm1.gain(ControlPCM186x::ADC1R));
  Serial.printf("ADC1L g=%.1fdB\n", pcm1.gain(ControlPCM186x::ADC1L));
  char gs[16];
  pcm1.gainStr(ControlPCM186x::ADC1L, gs, PREGAIN);
  setupStorage(gs);
  openNextFile();
}


void loop() {
  storeData();
  blink.update();
}
