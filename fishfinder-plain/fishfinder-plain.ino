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
int averaging = 1;             // number of averages per sample: 0, 4, 8, 16, 32 - the higher the better, but the slowe
uint32_t samplingRate = 96000; // samples per second and channel in Hertz
int8_t channel =  A14;         // input pin for ADC0

char fileName[] = "SDATEFNUM.wav";  // may include DATE, SDATE, TIME, STIME,

int ampl_enable_pin = 32;      // pin for enabling an audio amplifier
int volume_up_pin = 25;        // pin for push button for increasing audio volume
int volume_down_pin = 26;      // pin for push button for decreasing audio volume
int startPin = 24;             // pin for push button starting and stopping a recording

// ----------------------------------------------------------------------------

Configurator config;
Settings settings("recordings", fileName);

ContinuousADC aidata;

AudioPlayBuffer playdata(aidata);
AudioMixer4 mix;
AudioOutputI2S speaker;
AudioConnection ac1(playdata, 0, mix, 0);
AudioConnection aco(mix, 0, speaker, 0);
AudioControlSGTL5000 audioshield;

SDCard sdcard;
SDWriter file(sdcard, aidata);

RTClock rtclock;
String prevname; // previous file name
int restarts = 0;

PushButtons buttons;
float volume = 0.2;

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
  mix.gain(0, volume);
}


void volumeUp(int id) {
  volume *= 1.414213;
  if (volume > 0.8)
    volume = 0.8;
  mix.gain(0, volume);
}


void volumeDown(int id) {
  volume /= 1.414213;
  if (volume < 0.00625)
    volume = 0.00625;
  mix.gain(0, volume);
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


void setupButtons() {
  buttons.add(startPin, INPUT_PULLUP, startWrite);
  buttons.add(volume_up_pin, INPUT_PULLUP, volumeUp);
  buttons.add(volume_down_pin, INPUT_PULLUP, volumeDown);
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
  blink.switchOff();
}


void loop() {
  buttons.update();
  storeData();
  blink.update();
}
