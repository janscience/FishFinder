#include <Configurator.h>
#include <Settings.h>
#include <ContinuousADC.h>
#include <AudioPlayBuffer.h>
#include <SDWriter.h>
#include <RTClock.h>
#include <PushButtons.h>
#include <Blink.h>

// Default settings: ----------------------------------------------------------
// (may be overwritten by config file fishgrid.cfg)

int bits = 12;                 // resolution: 10bit 12bit, or 16bit 
int averaging = 1;             // number of averages per sample: 0, 4, 8, 16, 32 - the higher the better, but the slower
uint32_t samplingRate = 96000; // samples per second and channel in Hertz
int8_t channel =  A14;         // input pin for ADC0

char fileName[] = "SDATEFNUM.wav";  // may include DATE, SDATE, TIME, STIME,

int ampl_enable_pin = 32;      // pin for enabling an audio amplifier
int startPin = 24;             // pin for push button starting and stopping a recording

uint updateAnalysis = 500;     // milliseconds
float analysisWindow = 0.2;    // seconds

// ----------------------------------------------------------------------------

Configurator config;
Settings settings("recordings", fileName);

ContinuousADC aidata;

AudioPlayBuffer playdata(aidata);
AudioPlayMemory sound0;
AudioMixer4 mix;
AudioOutputI2S speaker;
AudioConnection ac1(playdata, 0, mix, 0);
AudioConnection ac2(sound0, 0, mix, 1);
AudioConnection aco(mix, 0, speaker, 0);
AudioControlSGTL5000 audioshield;
int16_t *Beep;
uint BeepInterval = 0;
elapsedMillis BeepTime;
elapsedMillis analysisTime;

SDCard sdcard;
SDWriter file(sdcard, aidata);

RTClock rtclock;
String prevname; // previous file name
int restarts = 0;

PushButtons buttons;
Blink blink(LED_BUILTIN);


void setupADC() {
  aidata.setChannel(0, channel);
  aidata.setRate(samplingRate);
  aidata.setResolution(bits);
  aidata.setAveraging(averaging);
  aidata.setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);
  aidata.setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED);
  aidata.setReference(ADC_REFERENCE::REF_3V3);
  aidata.check();
}


void setupAudio() {
  AudioMemory(16);
  if ( ampl_enable_pin >= 0 ) {
    pinMode(ampl_enable_pin, OUTPUT);
    digitalWrite(ampl_enable_pin, HIGH); // turn on the amplifier
    delay(10);                           // allow time to wake up
  }
  audioshield.enable();
  //audioshield.volume(0.5);
  //audioshield.muteHeadphone();
  //audioshield.muteLineout();
  audioshield.lineOutLevel(31);
  mix.gain(0, 0.1);
  mix.gain(1, 0.02);
  // make a beep:
  float freq = 4*440.0;
  float duration = 0.2;
  size_t np = size_t(AUDIO_SAMPLE_RATE_EXACT/freq);
  unsigned int n = (unsigned int)(duration*AUDIO_SAMPLE_RATE_EXACT/np)*np;
  Beep = new int16_t[2+n];
  // first integer encodes format and size:
  unsigned int format = 0x81;
  format <<= 24;
  format |= n;
  Beep[0] = format & 0xFFFF;
  Beep[1] = format >> 16;
  // Gabor sine tone:
  uint16_t a = 1 << 15;
  for (size_t i=0; i<n; i++)
    Beep[2+i] = (int16_t)(a*exp(-0.5*pow((i-n/2)/(n/4), 2))*sin(TWO_PI*i/np));
}


String makeFileName() {
  time_t t = now();
  String name = rtclock.makeStr(settings.FileName, t, true);
  if (name != prevname) {
    file.resetFileCounter();
    prevname = name;
  }
  name = file.incrementFileName(name);
  if (name.length() == 0) {
    Serial.println("WARNING: failed to increment file name.");
    Serial.println("SD card probably not inserted.");
    Serial.println();
    return "";
  }
  return name;
}


bool openNextFile(const String &name) {
  blink.clear();
  if (name.length() == 0)
    return false;
  char dts[20];
  rtclock.dateTime(dts);
  if (! file.openWave(name.c_str(), -1, dts)) {
    Serial.println();
    Serial.println("WARNING: failed to open file on SD card.");
    Serial.println("SD card probably not inserted or full -> halt");
    aidata.stop();
    while (1) {};
    return false;
  }
  file.write();
  Serial.println(name);
  blink.setSingle();
  blink.blinkSingle(0, 1000);
  return true;
}


void startWrite(int id) {
  // on button press:
  if (file.available()) {
    if (!file.isOpen()) {
      String name = makeFileName();
      if (name.length() == 0) {
        Serial.println("-> halt");
        aidata.stop();
        while (1) {};
      }
      file.setMaxFileSamples(0);
      file.start();
      openNextFile(name);
    }
    else {
      file.closeWave();
      blink.clear();
      Serial.println("  stopped recording\n");
    }
  }
}


void setupButtons() {
  buttons.add(startPin, INPUT_PULLUP, startWrite);
}


void setupStorage() {
  prevname = "";
  if (file.dataDir(settings.Path))
    Serial.printf("Save recorded data in folder \"%s\".\n\n", settings.Path);
  file.setWriteInterval();
  file.setSoftware("FishFinder plain");
}


void storeData() {
  if (file.pending()) {
    ssize_t samples = file.write();
    if (samples <= 0) {
      blink.clear();
      Serial.println();
      Serial.println("ERROR in writing data to file:");
      switch (samples) {
        case 0:
          Serial.println("  Nothing written into the file.");
          Serial.println("  SD card probably full -> halt");
          aidata.stop();
          while (1) {};
          break;
        case -1:
          Serial.println("  File not open.");
          break;
        case -2:
          Serial.println("  File already full.");
          break;
        case -3:
          Serial.println("  No data available, data acquisition probably not running.");
          Serial.println("  sampling rate probably too high,");
          Serial.println("  given the number of channels, averaging, sampling and conversion speed.");
          break;
      }
      if (samples == -3) {
        aidata.stop();
        file.closeWave();
        char mfs[20];
        sprintf(mfs, "error%d-%d.msg", restarts+1, -samples);
        FsFile mf = sdcard.openWrite(mfs);
        mf.close();
      }
    }
  }
}


void analyzeData() {
  if (analysisTime > updateAnalysis) {
    analysisTime -= updateAnalysis;
    size_t n = aidata.frames(analysisWindow);
    sample_t data[n];
    aidata.getData(0, aidata.currentSample(n), data, n);
    sample_t max = 1 << 14;
    int nover = 0;
    for (size_t i=0; i<n; i++) {
      if (data[i] > max || data[i] < -max)
        nover++;
    }
    float frac = float(nover)/n;
    if (frac > 0.0001)
      BeepInterval = 500 - frac*300;
    else
      BeepInterval = 0;
  }
}


// ---------------------------------------------------------------------------

void setup() {
  blink.switchOn();
  Serial.begin(9600);
  while (!Serial && millis() < 2000) {};
  rtclock.check();
  rtclock.report();
  setupButtons();
  setupADC();
  sdcard.begin();
  config.setConfigFile("fishfinder.cfg");
  config.configure(sdcard);
  setupStorage();
  aidata.check();
  setupAudio();
  aidata.start();
  aidata.report();
  analysisTime = 0;
  blink.switchOff();
}


void loop() {
  buttons.update();
  storeData();
  analyzeData();
  blink.update();
  if (BeepInterval > 0 && BeepTime > BeepInterval) {
    BeepTime -= BeepInterval;
    sound0.play((const unsigned int *)Beep);
  }
}
