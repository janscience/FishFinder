#include <Configurator.h>
#include <Settings.h>
#include <ContinuousADC.h>
#include <AudioMonitor.h>
#include <SDWriter.h>
#include <Display.h>
#include <ST7789_t3.h>
#include <fonts/FreeSans10pt7b.h>
#include <fonts/FreeSans11pt7b.h>
#include <fonts/FreeSans12pt7b.h>
#include <AnalysisChain.h>
#include <Clipping.h>
#include <Correlation.h>
#include <Plotting.h>
#include <ReportTime.h>
#include <RTClock.h>
#include <PushButtons.h>
#include <Blink.h>

// Default settings: ----------------------------------------------------------
// (may be overwritten by config file fishgrid.cfg)

int bits = 12;                 // resolution: 10bit 12bit, or 16bit 
int averaging = 4;             // number of averages per sample: 0, 4, 8, 16, 32 - the higher the better, but the slower
uint32_t samplingRate = 44100; // samples per second and channel in Hertz, 22.05kHz, 44.1kHz or 96kHz
int8_t channels [] =  {A14, A15, -1, A2, A3, A4, A5, A6, A7, A8, A9};      // input pins for ADC0, terminate with -1
			
char fileName[] = "SDATEFNUM.wav";  // may include DATE, SDATE, TIME, STIME,

#define AMPL_ENABLE_PIN  32  // pin for enabling an audio amplifier
#define VOLUME_UP_PIN    25  // pin for push button for increasing audio volume
#define VOLUME_DOWN_PIN  26  // pin for push button for decreasing audio volume
#define START_PIN        24  // pin for push button starting and stopping a recording

// define pins to control TFT display:
#define TFT_SCK   13   // default SPI0 bus
#define TFT_MISO  12   // default SPI0 bus
#define TFT_MOSI  11   // default SPI0 bus
#define TFT_CS    10  
#define TFT_RST   8 // 9
#define TFT_DC    7 // 8 
#define TFT_BL   30 // backlight PWM, -1 to not use it

float updateAnalysis = 0.25;    // seconds
float analysisWindow = 0.25;    // seconds


// ----------------------------------------------------------------------------

Configurator config;
Settings settings("recordings", fileName);

DATA_BUFFER(AIBuffer, NAIBuffer, 256*128)
ContinuousADC aidata(AIBuffer, NAIBuffer);

AudioOutputI2S speaker;
AudioMonitor audio(aidata, speaker);

Display screen;
ST7789_t3 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RST);

SDCard sdcard;
SDWriter file(sdcard, aidata);

RTClock rtclock;

AnalysisChain analysis(aidata);
Clipping clipping(&audio, &analysis);
Correlation correlation(&audio, &analysis);
Plotting plotting(&screen, &analysis);
ReportTime reporttime(&screen, 1, &rtclock, &analysis);

PushButtons buttons;
Blink blink(LED_BUILTIN);

String prevname; // previous file name
int restarts = 0;

#define SOFTWARE "FishFinder V1.0"

#define SCREEN_TEXT_ACTION 0
#define SCREEN_TEXT_DATETIME 1
#define SCREEN_TEXT_FILENAME 2
#define SCREEN_TEXT_FILETIME 3


void setupADC() {
  aidata.setChannels(0, channels);
  aidata.setRate(samplingRate);
  aidata.setResolution(bits);
  aidata.setAveraging(averaging);
  aidata.setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);
  aidata.setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED);
  aidata.setReference(ADC_REFERENCE::REF_3V3);
  aidata.check();
}


void setupAudio() {
  audio.setup(AMPL_ENABLE_PIN, 0.02, VOLUME_UP_PIN, VOLUME_DOWN_PIN);
  audio.addFeedback(0.05, 2*440.0, 0.2);
  audio.addFeedback(0.2, 440.0, 0.2);
}


void initScreen(Display &screen) {
  tft.init(240, 320);
  DisplayWrapper<ST7789_t3> *tftscreen = new DisplayWrapper<ST7789_t3>(&tft);
  screen.init(tftscreen, 1, true);
  Serial.println();
  screen.setDefaultFont(FreeSans12pt7b);
  screen.setTitleFont(FreeSans12pt7b);
  screen.setSmallFont(FreeSans10pt7b);
  screen.setBacklightPin(TFT_BL);
  screen.clear();
}


void AIsplashScreen(Display &screen,
		    const ContinuousADC &aidata, const char *title) {
  char msg[100];
  String convspeed = aidata.conversionSpeedShortStr();
  String samplspeed = aidata.samplingSpeedShortStr();
  char chans0[50];
  char chans1[50];
  aidata.channels(0, chans0);
  aidata.channels(1, chans1);
  if (chans0[0] == '\0')
    strcpy(chans0, "-");
  if (chans1[0] == '\0')
    strcpy(chans1, "-");
  float bt = aidata.bufferTime();
  char bts[20];
  if (bt < 1.0)
    sprintf(bts, "%.0fms\n", 1000.0*bt);
  else
    sprintf(bts, "%.2fs\n", bt);
  sprintf(msg, "%.0fkHz\n%dbit\n%d,%s,%s\n%s\n%s\n%s",
          0.001*aidata.rate(), aidata.resolution(), aidata.averaging(),
          convspeed.c_str(), samplspeed.c_str(), chans0, chans1, bts);
  screen.setTextArea(0, 0.0, 0.75, 1.0, 0.95);
  screen.setTitleFont(0);
  screen.setTextArea(1, 0.0, 0.0, 0.4, 0.7, true);
  screen.setSmallFont(1);
  screen.setTextArea(2, 0.4, 0.0, 1.0, 0.7, true);
  screen.setSmallFont(2);
  screen.writeText(0, title);
  screen.writeText(1, "rate:\nres.:\nspeed:\nADC0:\nADC1\nbuffer:");
  screen.writeText(2, msg);
  screen.fadeBacklightOn();
  delay(1500);
  screen.fadeBacklightOff();
  screen.clear();
  screen.clearText();
}


void setupScreen() {
  screen.setTextArea(SCREEN_TEXT_ACTION, 0.0, 0.85, 0.38, 1.0);   // action
  screen.setTextArea(SCREEN_TEXT_DATETIME, 0.4, 0.85, 1.0, 1.0);    // data&time
  screen.setTextArea(SCREEN_TEXT_FILENAME, 0.0, 0.7, 0.75, 0.85);   // file
  screen.setTextArea(SCREEN_TEXT_FILETIME, 0.8, 0.7, 1.0, 0.85);    // file time
  screen.setPlotAreas(1, 0.0, 0.0, 1.0, 0.7);
  screen.setBacklightOn();
}


void diskFull() {
  Serial.println("SD card probably not inserted or full");
  Serial.println();
  screen.writeText(SCREEN_TEXT_ACTION, "!NO SD CARD OR FULL!");
  screen.clearText(SCREEN_TEXT_FILENAME);
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
    diskFull();
    return "";
  }
  return name;
}


bool openFile(const String &name) {
  blink.clear();
  if (name.length() == 0)
    return false;
  char dts[20];
  rtclock.dateTime(dts);
  if (! file.openWave(name.c_str(), -1, dts)) {
    Serial.println("WARNING: failed to open file on SD card.");
    diskFull();
    return false;
  }
  file.setMaxFileSamples(0);
  file.start();
  // all screen writing 210ms:
  screen.writeText(SCREEN_TEXT_ACTION, "recording:");
  screen.writeText(SCREEN_TEXT_FILENAME, name.c_str());
  Serial.println(name);
  blink.setSingle();
  blink.blinkSingle(0, 1000);
  return true;
}


void startStopWrite(int id) {
  // on button press:
  if (file.isOpen()) {
    file.closeWave();
    blink.clear();
    screen.writeText(SCREEN_TEXT_ACTION, "last file:");
    screen.clearText(SCREEN_TEXT_FILETIME);
    Serial.println("  stopped recording\n");
  }
  else {
    String name = makeFileName();
    openFile(name);
  }
}


void setupButtons() {
  buttons.add(START_PIN, INPUT_PULLUP, startStopWrite);
}


void setupStorage() {
  prevname = "";
  if (file.dataDir(settings.Path))
    Serial.printf("Save recorded data in folder \"%s\".\n\n", settings.Path);
  file.setWriteInterval();
  file.setSoftware(SOFTWARE);
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
          Serial.println("  SD card probably full");
          screen.writeText(SCREEN_TEXT_ACTION, "!NOTHING WRITTEN!");
          break;
        case -3:
          Serial.println("  No data available, data acquisition probably not running.");
          Serial.println("  sampling rate probably too high,");
          Serial.println("  given the number of channels, averaging, sampling and conversion speed.");
          screen.writeText(SCREEN_TEXT_ACTION, "!NO DATA!");
          break;
        default: // -1 (file not open), -2 (file already full)
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
    if (file.isOpen()) {
      char ts[6];
      file.fileTimeStr(ts);
      screen.writeText(SCREEN_TEXT_FILETIME, ts);
    }
  }
}


void setupAnalysis() {
  clipping.disable();
  correlation.disable();
  clipping.setThreshold(0.75);   // make it configurable!
  plotting.setSkipping(2);
  plotting.setWindow(0.01);
  plotting.setAlignMax(0.5);
  analysis.start(updateAnalysis, analysisWindow);
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
  initScreen(screen);
  AIsplashScreen(screen, aidata, "FishFinder V1.0");
  setupScreen();
  setupAudio();
  setupAnalysis();
  aidata.start();
  aidata.report();
  blink.switchOff();
}


void loop() {
  buttons.update();
  storeData();
  analysis.update();
  audio.update();
  blink.update();
}
