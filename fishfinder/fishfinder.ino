#include <Configurator.h>
#include <Settings.h>
#include <ContinuousADC.h>
#include <AudioMonitor.h>
#include <SDWriter.h>
#include <Display.h>
#include <ST7789_t3.h>
#include <fonts/FreeSans12pt7b.h>
#include <AnalysisChain.h>
#include <Clipping.h>
//#include <Correlation.h>
#include <Spectrum.h>
#include <ReportPeakFreq.h>
#include <Plotting.h>
#include <ReportTime.h>
#include <RTClock.h>
#include <PushButtons.h>
#include <Blink.h>

//#define DEBUG

#define SOFTWARE "FishFinder V1.0"

// Default settings: ----------------------------------------------------------
// (may be overwritten by config file fishgrid.cfg)

uint32_t SamplingRate  = 44100;      // samples per second and channel in Hertz, 22.05kHz, 44.1kHz or 96kHz
uint8_t  Averaging     = 4;          // number of averages per sample: 0, 4, 8, 16, 32 - the higher the better, but the slower
uint8_t  Bits          = 12;         // resolution: 10bit 12bit, or 16bit 
ADC_CONVERSION_SPEED ConversionSpeed = ADC_CONVERSION_SPEED::HIGH_SPEED;
ADC_SAMPLING_SPEED   SamplingSpeed   = ADC_SAMPLING_SPEED::HIGH_SPEED;
			
#define FILENAME         "SDATEFNUM.wav" // may include DATE, SDATE, TIME, STIME, NUM, ANUM

#define DATA_BUFFER_SIZE 256*96
#define AUDIO_BLOCKS     4

#define UPDATE_ANALYSIS  0.2 // seconds
#define ANALYSIS_WINDOW  0.2 // seconds

#define MAX_TEXT_SWAP 5
#define MAX_FILE_SHOWTIME 30*1000 // 30s

// Pin assignment: ------------------------------------------------------------

//#define CHANNEL_FRONT    A10 // input pin for front electrode
#define CHANNEL_FRONT    A2 // input pin for front electrode
#define CHANNEL_BACK     A2  // input pin for back electrode

#define CHANNEL_VOICE    A0  // input pin for voice message

#define AMPL_ENABLE_PIN   7  // pin for enabling an audio amplifier
#define DOWN_PIN         26  // pin for push button for decreasing audio volume/zoom
#define UP_PIN           27  // pin for push button for increasing audio volume/zoom
#define RECORD_PIN       28  // pin for push button starting and stopping a recording
#define VOICE_PIN        29  // pin for push button starting and stopping a voice message

// LEDs: ----------------------------------------------------------------------

#define RECORD_LED_PIN   11
#define VOICE_LED_PIN    12

// TFT display: ---------------------------------------------------------------

#define TFT_ROTATION      3

#define TFT_SCK_PIN      32   // SPI1 bus
#define TFT_CS_PIN       31  
#define TFT_MOSI_PIN      0   // SPI1 bus
#define TFT_RST_PIN       1
#define TFT_DC_PIN       15
#define TFT_BL_PIN       30   // backlight PWM

// Text areas: ----------------------------------------------------------------

#define SCREEN_TEXT_ACTION    0
#define SCREEN_TEXT_DATEFILE  1
#define SCREEN_TEXT_PEAKFREQ  2
#define SCREEN_TEXT_FILETIME  3
#define SCREEN_TEXT_UPDOWN    4
#define SCREEN_TEXT_CLIPPING  5
#define SCREEN_TEXT_TIME      6

// ----------------------------------------------------------------------------

Configurator config;
Settings settings("recordings", FILENAME);

DATA_BUFFER(AIBuffer, NAIBuffer, DATA_BUFFER_SIZE);
ContinuousADC aidata(AIBuffer, NAIBuffer);

AudioOutputI2S speaker;
AudioMonitor audio(aidata, speaker);

Display screen;
ST7789_t3 tft(TFT_CS_PIN, TFT_DC_PIN, TFT_MOSI_PIN,
              TFT_SCK_PIN, TFT_RST_PIN);

SDCard sdcard;
SDWriter datafile(sdcard, aidata);
SDWriter voicefile(sdcard, aidata);
int SwapCounter;
elapsedMillis DateFileTime;

RTClock rtclock;

AnalysisChain analysis(aidata);
Clipping clipping(0, &audio, 0, &analysis);
//Correlation correlation(&audio, 1, &analysis);
Spectrum spectrum(0, &analysis);
ReportPeakFreq peakfreq(&spectrum, &screen, SCREEN_TEXT_PEAKFREQ, &analysis);
Plotting plotting(0, 0, &screen, 0, SCREEN_TEXT_TIME, &analysis);
ReportTime reporttime(&screen, SCREEN_TEXT_DATEFILE, &rtclock, &analysis);

PushButtons buttons;
Blink blink(RECORD_LED_PIN);
Blink voiceled(VOICE_LED_PIN);

String prevname; // previous file name without increment
String lastname; // last recorded file name
int restarts = 0;
int updownstate = 0;    // how to use up/down buttons
const int maxupdownstates = 2; // number of different usages for up/down buttons
char updownids[maxupdownstates][2] = {"V", "X"};
char clippingids[maxupdownstates][2] = {"", "C"};

#ifdef DEBUG
#include <FreeStack.h>
  #define CHECK_MEMORY Serial.printf("Free Memory: %d\n", FreeStack());
#else
  #define CHECK_MEMORY 
#endif


void setupDataADC() {
  aidata.clearChannels();
  aidata.setChannel(0, CHANNEL_FRONT);
  //aidata.setChannel(1, CHANNEL_BACK);
  aidata.setRate(SamplingRate);
  aidata.setResolution(Bits);
  aidata.setAveraging(Averaging);
  aidata.setConversionSpeed(ConversionSpeed);
  aidata.setSamplingSpeed(SamplingSpeed);
  aidata.setReference(ADC_REFERENCE::REF_3V3);
  aidata.check();
}


void setupVoiceADC() {
  aidata.clearChannels();
  aidata.setChannel(0, CHANNEL_VOICE);
  aidata.setRate(20050.0);
  aidata.setAveraging(4);
  aidata.check();
}


void initScreen(Display &screen) {
  tft.init(240, 320);
  DisplayWrapper<ST7789_t3> *tftscreen = new DisplayWrapper<ST7789_t3>(&tft);
  screen.init(tftscreen, TFT_ROTATION, true);
  Serial.println();
  screen.setDefaultFont(FreeSans12pt7b);
  screen.clear();
}


void setupScreen() {
  screen.setTextArea(SCREEN_TEXT_ACTION, 0.0, 0.87, 0.38, 1.0);
  screen.setTextArea(SCREEN_TEXT_DATEFILE, 0.4, 0.87, 1.0, 1.0);
  screen.setTextArea(SCREEN_TEXT_PEAKFREQ, 0.0, 0.72, 0.3, 0.87);
  screen.setTextArea(SCREEN_TEXT_FILETIME, 0.8, 0.72, 1.0, 0.87);
  screen.setTextArea(SCREEN_TEXT_UPDOWN, 0.95, 0.79, 1.0, 0.87, true);
  screen.setTextArea(SCREEN_TEXT_CLIPPING, 0.89, 0.79, 0.94, 0.87, true);
  screen.setTextArea(SCREEN_TEXT_TIME, 0.0, 0.0, 0.25, 0.13);
  screen.swapTextColors(SCREEN_TEXT_UPDOWN);
  screen.swapTextColors(SCREEN_TEXT_CLIPPING);
  screen.writeText(SCREEN_TEXT_UPDOWN, updownids[updownstate]);
  screen.writeText(SCREEN_TEXT_CLIPPING,
                   clippingids[clipping.feedbackEnabled()]);
  screen.setPlotAreas(1, 0.0, 0.0, 1.0, 0.72);
  screen.setBacklightOn();
}


void diskFull() {
  Serial.println("SD card probably not inserted or full");
  Serial.println();
  screen.writeText(SCREEN_TEXT_ACTION, "!NO SD CARD!");
}


String makeFileName() {
  CHECK_MEMORY
  time_t t = now();
  String name = rtclock.makeStr(settings.FileName, t, true);
  if (name != prevname) {
    datafile.resetFileCounter();
    prevname = name;
  }
  name = datafile.incrementFileName(name);
  if (name.length() <= 0) {
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
  if (! datafile.openWave(name.c_str(), -1, dts)) {
    Serial.println("WARNING: failed to open file on SD card.");
    diskFull();
    return false;
  }
  lastname = name;
  datafile.setMaxFileSamples(0);
  datafile.start();
  // all screen writing 210ms:
  screen.writeText(SCREEN_TEXT_ACTION, "REC");
  screen.writeText(SCREEN_TEXT_DATEFILE, name.c_str());
  Serial.println(name);
  blink.setSingle();
  blink.blinkSingle(0, 1000);
  return true;
}


void toggleRecord(int id) {
  if (voicefile.isOpen())  // voice message in progress
    return;
  // on button press:
  if (datafile.isOpen()) {
    datafile.write();
    datafile.closeWave();
    blink.clear();
    screen.setDefaultTextColors(SCREEN_TEXT_ACTION);
    screen.clearText(SCREEN_TEXT_ACTION);
    screen.clearText(SCREEN_TEXT_FILETIME);
    Serial.println("  stopped recording\n");
  }
  else {
    reporttime.disable();
    String name = makeFileName();
    openFile(name);
    SwapCounter = 0;
  }
  DateFileTime = 0;
}


void toggleVoiceMessage(int id) {
  if (reporttime.enabled()) {
    if (buttons.button(id)->previousDuration() > 500) {
      clipping.toggleFeedback();
      screen.writeText(SCREEN_TEXT_CLIPPING,
                       clippingids[clipping.feedbackEnabled()]);
    }
    else {
      // change up/down switch usage:
      updownstate++;
      if (updownstate >= maxupdownstates)
        updownstate = 0;
      screen.writeText(SCREEN_TEXT_UPDOWN, updownids[updownstate]);
    }
  }
  else {
    // voice message only as long last file name is shown:
    if (datafile.isOpen())       // recording in progress
      return;
    if (lastname.length() == 0)  // no recording yet
      return;
    if (voicefile.isOpen()) {    // stop voice message
      voicefile.write();
      voicefile.closeWave();
      voiceled.clear();
      aidata.stop();
      screen.setDefaultTextColors(SCREEN_TEXT_ACTION);
      screen.clearText(SCREEN_TEXT_ACTION);
      screen.clearText(SCREEN_TEXT_FILETIME);
      setupDataADC();
      aidata.start();
      aidata.report();
      audio.play();
      analysis.start(UPDATE_ANALYSIS, ANALYSIS_WINDOW);
      Serial.println("STOP VOICE MESSAGE");
    }
    else {                       // start voice message
      analysis.stop();
      audio.pause();
      aidata.stop();
      screen.clearPlots();
      setupVoiceADC();
      aidata.start();
      aidata.report();
      String voicename = lastname;
      voicename.remove(voicename.indexOf(".wav"));
      voicename += "-message.wav";
      voicefile.openWave(voicename.c_str());
      voicefile.setMaxFileSamples(0);
      voicefile.start();
      screen.writeText(SCREEN_TEXT_ACTION, "VOICE");
      screen.clearText(SCREEN_TEXT_PEAKFREQ);
      voiceled.setSingle();
      voiceled.blinkSingle(0, 1000);
      Serial.println("START VOICE MESSAGE");
      SwapCounter = 0;
    }
    DateFileTime = 0;
  }
}


void switchUp(int id) {
  switch (updownstate) {
  case 1:
    plotting.zoomIn();
    break;
  default:
    audio.volumeUp();
  }
}


void switchDown(int id) {
  switch (updownstate) {
  case 1:
    plotting.zoomOut();
    break;
  default:
    audio.volumeDown();
  }
}


void setupButtons() {
  buttons.add(RECORD_PIN, INPUT_PULLUP, 0, toggleRecord);
  buttons.add(VOICE_PIN, INPUT_PULLUP, 0, toggleVoiceMessage);
  buttons.add(UP_PIN, INPUT_PULLUP, switchUp);
  buttons.add(DOWN_PIN, INPUT_PULLUP, switchDown);
}


void setupStorage() {
  prevname = "";
  lastname = "";
  if (datafile.dataDir(settings.Path))
    Serial.printf("Save recorded data in folder \"%s\".\n\n", settings.Path);
  datafile.setWriteInterval();
  datafile.setSoftware(SOFTWARE);
}


void storeData() {
  bool swap = false;
  if (datafile.pending()) {
    ssize_t samples = datafile.write();
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
        datafile.closeWave();
        char mfs[20];
        sprintf(mfs, "error%d-%d.msg", restarts+1, -samples);
        FsFile mf = sdcard.openWrite(mfs);
        mf.close();
      }
    }
    if (datafile.isOpen()) {
      char ts[6];
      datafile.fileTimeStr(ts);
      screen.writeText(SCREEN_TEXT_FILETIME, ts);
      swap = true;
    }
  }
  if (voicefile.pending()) {
    voicefile.write();
    char ts[6];
    voicefile.fileTimeStr(ts);
    screen.writeText(SCREEN_TEXT_FILETIME, ts);
    swap = true;
  }
  if (swap) {
    SwapCounter++;
    if (SwapCounter >= MAX_TEXT_SWAP) {
      SwapCounter = 0;
      screen.swapTextColors(SCREEN_TEXT_ACTION);
      screen.rewriteText(SCREEN_TEXT_ACTION);
    }
    DateFileTime = 0;
  }
}


void setupAudio() {
  //audio.setMixer(&AudioPlayBuffer::difference);
  audio.setMixer(&AudioPlayBuffer::assign);
  AudioMemory(AUDIO_BLOCKS);
  audio.setupAmp(AMPL_ENABLE_PIN);
  audio.setupVolume(0.02);
  audio.setLowpass(2);
  audio.addFeedback(0.4, 6*440.0, 0.17);
  //audio.addFeedback(0.2, 2*440.0, 0.2);
}


void setupAnalysis() {
  //clipping.disable();
  //correlation.disable();
  //clipping.setClipThreshold(0.9);   // make it configurable!
  clipping.setClipThreshold(0.8);   // make it configurable!
  clipping.setMuteThreshold(0.7);   // make it configurable!
  spectrum.setNFFT(4096);
  spectrum.setResolution(3.0);
  plotting.setSkipping(4);
  plotting.setWindow(0.01);
  plotting.setAlignMax(0.5);
  analysis.start(UPDATE_ANALYSIS, ANALYSIS_WINDOW);
}


// ---------------------------------------------------------------------------

void setup() {
  screen.setBacklightPin(TFT_BL_PIN);
  blink.switchOn();
  voiceled.switchOn();
  Serial.begin(9600);
  while (!Serial && millis() < 200) {};
  rtclock.check();
  rtclock.report();
  setupButtons();
  setupDataADC();
  sdcard.begin();
  config.setConfigFile("fishfinder.cfg");
  config.configure(sdcard);
  SamplingRate = aidata.rate();
  Averaging = aidata.averaging();
  Bits = aidata.resolution();
  ConversionSpeed = aidata.conversionSpeed();
  SamplingSpeed = aidata.samplingSpeed();
  setupStorage();
  aidata.check();
  initScreen(screen);
  DateFileTime = 0;
  setupScreen();
  setupAudio();
  setupAnalysis();
  aidata.start();
  aidata.report();
  blink.switchOff();
  voiceled.switchOff();
}


void loop() {
  buttons.update();
  storeData();
  analysis.update();
  audio.update();
  blink.update();
  if (DateFileTime > MAX_FILE_SHOWTIME) {
    reporttime.enable();
    screen.writeText(SCREEN_TEXT_CLIPPING,
                     clippingids[clipping.feedbackEnabled()]);
    screen.writeText(SCREEN_TEXT_UPDOWN, updownids[updownstate]);
 }
}
