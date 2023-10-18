#include <Wire.h>
#include <ControlPCM186x.h>
#include <InputTDM.h>
#include <SDWriter.h>
#include <RTClock.h>
#include <Blink.h>
#include <Configurator.h>
#include <Settings.h>
#include <InputTDMSettings.h>
#include <FileStorage.h>

// Default settings: ----------------------------------------------------------
// (may be overwritten by config file logger.cfg)
#define NCHANNELS     2        // number of channels (even, from 2 to 16)
#define PREGAIN       10.0     // gain factor of preamplifier
#define SAMPLING_RATE 48000    // samples per second and channel in Hertz
#define GAIN          20.0     // dB

#define PATH          "recordings"   // folder where to store the recordings
#define FILENAME      "rec1-SDATETIME.wav"  // may include DATE, SDATE, TIME, STIME, DATETIME, SDATETIME, ANUM, NUM
#define FILE_SAVE_TIME 60    // seconds
#define INITIAL_DELAY  10.0  // seconds

// ----------------------------------------------------------------------------

#define LED_PIN        LED_BUILTIN   // 26

#define SOFTWARE      "R4.2 FishRecorder v1.0"

EXT_DATA_BUFFER(AIBuffer, NAIBuffer, 16*512*256)
InputTDM aidata(AIBuffer, NAIBuffer);
ControlPCM186x pcm1(Wire1, PCM186x_I2C_ADDR1, InputTDM::TDM2);
ControlPCM186x pcm2(Wire1, PCM186x_I2C_ADDR2, InputTDM::TDM2);

SDCard sdcard;
SDWriter file(sdcard, aidata);

Configurator config;
Settings settings(PATH, FILENAME, FILE_SAVE_TIME, 0.0,
                  0.0, INITIAL_DELAY);
InputTDMSettings aisettings(&aidata, SAMPLING_RATE, NCHANNELS, GAIN);                  
RTClock rtclock;
Blink blink(LED_PIN);


bool setupPCM(InputTDM &tdm, ControlPCM186x &pcm) {
  pcm.begin();
  bool r = pcm.setMicBias(false, true);
  if (!r) {
    Serial.println("not available");
    return false;
  }
  pcm.setRate(tdm, aisettings.rate());
  pcm.setupTDM(tdm, ControlPCM186x::CH2L, ControlPCM186x::CH2R,
               offs, ControlPCM186x::INVERTED);
  Serial.println("configured for 2 channels");
  pcm.setSmoothGainChange(false);
  pcm.setGain(aisettings.gain());
  pcm.setFilters(ControlPCM186x::FIR, false);
  return true;
}


// -----------------------------------------------------------------------------

void setup() {
  blink.switchOn();
  Serial.begin(9600);
  while (!Serial && millis() < 2000) {};
  rtclock.check();
  sdcard.begin();
  rtclock.setFromFile(sdcard);
  rtclock.report();
  config.setConfigFile("logger.cfg");
  config.configure(sdcard);
  aidata.setSwapLR();
  Wire1.begin();
  Serial.printf("Setup PCM186x 1 on TDM 2: ");
  setupPCM(aidata, pcm1);
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
  char gs[16];
  pcm->gainStr(gs, PREGAIN);
  setupStorage(SOFTWARE, aidata, gs);
  openNextFile();
}


void loop() {
  storeData();
  blink.update();
}
