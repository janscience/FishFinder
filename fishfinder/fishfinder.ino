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
#include <Analyzer.h>
#include <RTClock.h>
#include <PushButtons.h>
#include <Blink.h>

// Default settings: ----------------------------------------------------------
// (may be overwritten by config file fishgrid.cfg)

int bits = 12;                 // resolution: 10bit 12bit, or 16bit 
int averaging = 4;             // number of averages per sample: 0, 4, 8, 16, 32 - the higher the better, but the slower
uint32_t samplingRate = 44000; // samples per second and channel in Hertz
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

float updateAnalysis = 0.3;    // seconds
float analysisWindow = 0.1;    // seconds

// ----------------------------------------------------------------------------

Configurator config;
Settings settings("recordings", fileName);

ContinuousADC aidata;

AudioOutputI2S speaker;
AudioMonitor audio(aidata, speaker);

Display screen;
ST7789_t3 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RST);

SDCard sdcard;
SDWriter file(sdcard, aidata);

Analyzer analysis(aidata);

RTClock rtclock;
String prevname; // previous file name
int restarts = 0;

PushButtons buttons;
Blink blink(LED_BUILTIN);


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
  screen.setTextArea(0, 0.0, 0.85, 0.38, 1.0);   // action
  screen.setTextArea(1, 0.4, 0.85, 1.0, 1.0);    // data&time
  screen.setTextArea(2, 0.0, 0.7, 0.75, 0.85);   // file
  screen.setTextArea(3, 0.8, 0.7, 1.0, 0.85);    // file time
  screen.setPlotAreas(1, 0.0, 0.0, 1.0, 0.7);
  screen.setBacklightOn();
}


void plotData(const float *data, int n) {
  char ts[20];
  rtclock.dateTime(ts);
  ts[strlen(ts)-3] = '\0';
  screen.writeText(1, ts);
  screen.clearPlots();
  file.write();
  screen.plot(0, data, n, 0);
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


bool openFile(const String &name) {
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
  file.setMaxFileSamples(0);
  file.start();
  // all screen writing 210ms:
  screen.writeText(0, "recording:");
  screen.clearText(2);
  screen.writeText(2, name.c_str());
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
      openFile(name);
    }
    else {
      file.closeWave();
      blink.clear();
      screen.writeText(0, "last file:");
      screen.clearText(3);
      Serial.println("  stopped recording\n");
    }
  }
}


void setupButtons() {
  buttons.add(START_PIN, INPUT_PULLUP, startWrite);
}


void setupStorage() {
  prevname = "";
  if (file.dataDir(settings.Path))
    Serial.printf("Save recorded data in folder \"%s\".\n\n", settings.Path);
  file.setWriteInterval();
  file.setSoftware("FishFinder");
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
    if (file.isOpen()) {
      char ts[6];
      file.fileTimeStr(ts);
      screen.writeText(3, ts);
    }
  }
}


void plotting(float **data, uint8_t nchannels, size_t nsamples, float rate) {
  int nn = nsamples/5;
  float data_diff[nsamples];
  arm_add_f32(data[0], data[1], data_diff, nn);
  arm_scale_f32(data_diff, 0.5, data_diff, nn);
  plotData(data_diff, nn);
  //plotData(data[0], nn);
}


void clipping(float **data, uint8_t nchannels, size_t nsamples, float rate) {
  float max_val = 0.75;
  int nover = 0;
  for (uint8_t c=0; c<nchannels; c++) {
    for (size_t i=0; i<nsamples; i++) {
      if (data[c][i] > max_val || data[c][i] < -max_val)
	nover++;
    }
  }
  float frac = float(nover)/nsamples/nchannels;
  if (frac > 0.0001)
    audio.setFeedbackInterval(500 - frac*300, 0);
  else
    audio.setFeedbackInterval(0, 0);
};


void correlation(float **data, uint8_t nchannels, size_t nsamples, float rate) {
  // mean:
  float mean0 = 0.0;
  float mean1 = 0.0;
  arm_mean_f32(data[0], nsamples, &mean0);
  arm_mean_f32(data[1], nsamples, &mean1);
  // subtract mean:
  arm_offset_f32(data[0], -mean0, data[0], nsamples);
  arm_offset_f32(data[1], -mean1, data[1], nsamples);
  // standard deviations:
  float std0;
  float std1;
  arm_rms_f32(data[0], nsamples, &std0);
  arm_rms_f32(data[1], nsamples, &std1);
  // covariance:
  float d12[nsamples];
  arm_mult_f32(data[0], data[1], d12, nsamples);
  float covar;
  arm_mean_f32(d12, nsamples, &covar);
  float corr = covar;
  corr /= std0*std1;
  // costs:
  // cost <0: bad, 1: perfect
  // float costcorr = (0.2 - corr)/0.8;
  float costcorr = (corr - 0.2)/0.8;
  float costratio = 1.0 - abs(std0-std1)/(std0+std1);
  //float costampl = 0.5*(std0 + std1)/0.7; // better some maxima
  float cost = (costcorr + costratio)/2;
  //float cost = (0.2 - corr)/0.8;  // <0: bad, 1: perfect
  //Serial.printf("%6.3f  %6.3f  %6.3f  %6.3f\n", corr, costcorr, costratio, cost);
  if (cost < 0.0)
    audio.setFeedbackInterval(0, 1);
  else 
    audio.setFeedbackInterval(500 - cost*300, 1);
}


void setupAnalysis() {
  analysis.add(plotting);
  analysis.add(clipping);
  analysis.add(correlation);
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
  //analyzeData();
  analysis.update();
  audio.update();
  blink.update();
}
