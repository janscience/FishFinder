// Features:

// Make files on SD disk available via USB:
#define MTP_RESPONDER
// git clone git@github.com:KurtE/MTP_Teensy.git
// Requires Teensyduino >=1.57, set USB Type to "Serial + MTP Disk"

// Add logger mode:
#define LOGGER

// Provide menu entries for setting up ADC:
#define ADC_SETUP

// Provide menu entry for formatting SD card:
#define SDFORMAT

// Store selection in EEPROM:
#define STORE_SETUP

// Detect clipping and give audio feedback:
#define DETECT_CLIPPING

// Compute spectrum and detect paek frequency:
#define COMPUTE_SPECTRUM

//#define COMPUTE_CORRELATIONS    // TODO: not fully implemented yet

#include <TeensyADC.h>
#include <AudioMonitor.h>
#include <SDWriter.h>
#ifdef MTP_RESPONDER
#include <MTP_Teensy.h>
#endif
#include <Display.h>
#include <ST7789_t3.h>
#include <fonts/FreeSans12pt7b.h>
#include <AnalysisChain.h>
#ifdef DETECT_CLIPPING
#include <Clipping.h>
#endif
#ifdef COMPUTE_CORRELATIONS
#include <Correlation.h>
#endif
#ifdef COMPUTE_SPECTRUM
#include <Spectrum.h>
#include <ReportPeakFreq.h>
#endif
#include <Plotting.h>
#include <ReportTime.h>
#include <RTClock.h>
#include <PushButtons.h>
#include <Blink.h>
#include <Menu.h>
#include <Configurator.h>
#include <DeviceSettings.h>
#include <TeensyADCSettings.h>
#include <FishfinderADCSettings.h>
#include <FishfinderSettings.h>
#ifdef LOGGER
#include <LoggerSettings.h>
#endif
#ifdef STORE_SETUP
#include <EEPROM.h>
#endif

#define DEBUG

#define SOFTWARE "FishFinder V1.2"

// Default settings: ----------------------------------------------------------
// (may be overwritten by config file fishgrid.cfg)

#define BITS             12 // resolution: 10bit 12bit, or 16bit
#define REFERENCE     ADC_REFERENCE::REF_3V3

// ADC 1-channel @ 22kHz:
#define ADC1CH22KHZ_SAMPLING_RATE 22050 // samples per second and channel in Hertz, 22.05kHz, 44.1kHz, 96kHz, or 192kHz
#define ADC1CH22KHZ_AVERAGING         4 // number of averages per sample: 0, 4, 8, 16, 32
#define ADC1CH22KHZ_CONVERSION    ADC_CONVERSION_SPEED::HIGH_SPEED
#define ADC1CH22KHZ_SAMPLING      ADC_SAMPLING_SPEED::HIGH_SPEED
#define ADC1CH22KHZ_ANALYSIS_INTERVAL  0.2 // seconds
#define ADC1CH22KHZ_ANALYSIS_WINDOW  0.2 // seconds

// ADC 1-channel @ 44kHz:
#define ADC1CH44KHZ_SAMPLING_RATE 44100 // samples per second and channel in Hertz, 22.05kHz, 44.1kHz, 96kHz, or 192kHz
#define ADC1CH44KHZ_AVERAGING         4 // number of averages per sample: 0, 4, 8, 16, 32
#define ADC1CH44KHZ_CONVERSION    ADC_CONVERSION_SPEED::HIGH_SPEED
#define ADC1CH44KHZ_SAMPLING      ADC_SAMPLING_SPEED::HIGH_SPEED
#define ADC1CH44KHZ_ANALYSIS_INTERVAL  0.2 // seconds
#define ADC1CH44KHZ_ANALYSIS_WINDOW  0.2 // seconds

// ADC 1-channel @ 96kHz:
#define ADC1CH96KHZ_SAMPLING_RATE 96000 // samples per second and channel in Hertz, 22.05kHz, 44.1kHz, 96kHz, or 192kHz
#define ADC1CH96KHZ_AVERAGING         1 // number of averages per sample: 0, 4, 8, 16, 32
#define ADC1CH96KHZ_CONVERSION    ADC_CONVERSION_SPEED::HIGH_SPEED
#define ADC1CH96KHZ_SAMPLING      ADC_SAMPLING_SPEED::HIGH_SPEED
#define ADC1CH96KHZ_ANALYSIS_INTERVAL  0.2 // seconds
#define ADC1CH96KHZ_ANALYSIS_WINDOW  0.2 // seconds

// ADC 1-channel @ 192kHz:
#define ADC1CH192KHZ_SAMPLING_RATE 192000 // samples per second and channel in Hertz, 22.05kHz, 44.1kHz, 96kHz, or 192kHz
#define ADC1CH192KHZ_AVERAGING         1 // number of averages per sample: 0, 4, 8, 16, 32
#define ADC1CH192KHZ_CONVERSION    ADC_CONVERSION_SPEED::HIGH_SPEED
#define ADC1CH192KHZ_SAMPLING      ADC_SAMPLING_SPEED::HIGH_SPEED
#define ADC1CH192KHZ_ANALYSIS_INTERVAL  0.0 // seconds
#define ADC1CH192KHZ_ANALYSIS_WINDOW  0.0 // seconds

#define VOICE_SAMPLING_RATE 22050 // samples per second and channel in Hertz, 22.05kHz, 44.1kHz 96kHz, or 192kHz
#define VOICE_AVERAGING         4 // number of averages per sample: 0, 4, 8, 16, 32
#define VOICE_CONVERSION    ADC_CONVERSION_SPEED::HIGH_SPEED
#define VOICE_SAMPLING      ADC_SAMPLING_SPEED::HIGH_SPEED

#define DEVICE_NAME   "1"             // Name of device for file names.

#define PATH          "fishfinder"    // folder where to store recordings
#define FILENAME      "ffDEV-SDATENNUM.wav" // may include DEV, DATE, SDATE, TIME, STIME, NUM, ANUM

#ifdef LOGGER
#define LOGGER_PATH          "logger" // folder where to store logger recordings
#define LOGGER_FILENAME      "loggerDEV-SDATETIME" // may include DEV, DATE, SDATE, TIME, STIME, DATETIME, SDATETIME, ANUM, NUM
#define LOGGER_FILESAVETIME  1*60          // seconds
#define LOGGER_INITIALDELAY  1.0           // seconds
#endif

#define DATA_BUFFER_SIZE 256*96
#define AUDIO_BLOCKS     4

#define MAX_TEXT_SWAP 5
#define MAX_FILE_SHOWTIME 30*1000 // 30s

#ifdef COMPUTE_SPECTRUM
#define SPECTRUM_FMIN  70.0   // Hz
#define SPECTRUM_FMAX  2500.0 // Hz, 1.0e9 or larger: take all upto Nyquist frequency.
#endif

// Pin assignment: ------------------------------------------------------------

#define CHANNEL_FRONT    A10 // input pin for front electrode
//#define CHANNEL_FRONT    A2 // input pin for front electrode
//#define CHANNEL_BACK     A2  // input pin for back electrode

#define CHANNEL_VOICE    A0  // input pin for voice message

#define AMPL_ENABLE_PIN   7  // pin for enabling an audio amplifier
#define DOWN_PIN         26  // pin for push button for decreasing audio volume/zoom
#define UP_PIN           27  // pin for push button for increasing audio volume/zoom
#define RECORD_PIN       28  // pin for push button starting and stopping a recording
#define VOICE_PIN        29  // pin for push button starting and stopping a voice message

// LEDs: ----------------------------------------------------------------------

//#define RECORD_LED_PIN   11
//#define VOICE_LED_PIN    12
#define RECORD_LED_PIN   -1
#define VOICE_LED_PIN    -1

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
#define SCREEN_TEXT_AMPLITUDE 7

// ----------------------------------------------------------------------------

DATA_BUFFER(AIBuffer, NAIBuffer, DATA_BUFFER_SIZE);
TeensyADC aidata(AIBuffer, NAIBuffer);

AudioOutputI2S speaker;
AudioMonitor audio(aidata, speaker);

Display screen;
ST7789_t3 tft(TFT_CS_PIN, TFT_DC_PIN, TFT_MOSI_PIN,
              TFT_SCK_PIN, TFT_RST_PIN);

#ifdef MTP_RESPONDER
extern volatile uint8_t usb_configuration;
#endif
SDCard sdcard;
SDWriter datafile(sdcard, aidata);
SDWriter voicefile(sdcard, aidata);
int SwapCounter;
elapsedMillis DateFileTime;

RTClock rtclock;

Configurator config;
DeviceSettings device_settings(DEVICE_NAME);
FishfinderADCSettings ai1ch22khz_settings("1-Channel @ 22.05kHz",
					  ADC1CH22KHZ_SAMPLING_RATE,
					  BITS, ADC1CH22KHZ_AVERAGING,
					  ADC1CH22KHZ_CONVERSION,
					  ADC1CH22KHZ_SAMPLING, REFERENCE,
					  ADC1CH22KHZ_ANALYSIS_INTERVAL,
					  ADC1CH22KHZ_ANALYSIS_WINDOW);
FishfinderADCSettings ai1ch44khz_settings("1-Channel @ 44.1kHz",
					  ADC1CH44KHZ_SAMPLING_RATE,
					  BITS, ADC1CH44KHZ_AVERAGING,
					  ADC1CH44KHZ_CONVERSION,
					  ADC1CH44KHZ_SAMPLING, REFERENCE,
					  ADC1CH44KHZ_ANALYSIS_INTERVAL,
					  ADC1CH44KHZ_ANALYSIS_WINDOW);
FishfinderADCSettings ai1ch96khz_settings("1-Channel @ 96kHz",
					  ADC1CH96KHZ_SAMPLING_RATE,
					  BITS, ADC1CH96KHZ_AVERAGING,
					  ADC1CH96KHZ_CONVERSION,
					  ADC1CH96KHZ_SAMPLING, REFERENCE,
					  ADC1CH96KHZ_ANALYSIS_INTERVAL,
					  ADC1CH96KHZ_ANALYSIS_WINDOW);
FishfinderADCSettings ai1ch192khz_settings("1-Channel @ 192kHz",
					   ADC1CH192KHZ_SAMPLING_RATE,
					   BITS, ADC1CH192KHZ_AVERAGING,
					   ADC1CH192KHZ_CONVERSION,
					   ADC1CH192KHZ_SAMPLING, REFERENCE,
					   ADC1CH192KHZ_ANALYSIS_INTERVAL,
					   ADC1CH192KHZ_ANALYSIS_WINDOW);
#define MAX_SETTINGS 4
FishfinderADCSettings *ai_settings[MAX_SETTINGS] = { &ai1ch22khz_settings,
						     &ai1ch44khz_settings,
						     &ai1ch96khz_settings,
						     &ai1ch192khz_settings};
FishfinderSettings settings(PATH, FILENAME, 1);
#ifdef LOGGER
LoggerSettings logger_settings(LOGGER_PATH, LOGGER_FILENAME, 1,
			       LOGGER_FILESAVETIME, LOGGER_INITIALDELAY);
#endif
TeensyADCSettings voice_settings("Voice ADC", VOICE_SAMPLING_RATE, BITS,
				 VOICE_AVERAGING, VOICE_CONVERSION,
				 VOICE_SAMPLING, REFERENCE);

AnalysisChain analysis(aidata);
#ifdef DETECT_CLIPPING
Clipping clipping(0, &audio, 0, &analysis);
#endif
#ifdef COMPUTE_CORRELATIONS
Correlation correlation(&audio, 1, &analysis);
#endif
#ifdef COMPUTE_SPECTRUM
Spectrum spectrum(0, &analysis);
ReportPeakFreq peakfreq(&spectrum, &screen, SCREEN_TEXT_PEAKFREQ, &analysis);
#endif
Plotting plotting(0, 0, &screen, 0, SCREEN_TEXT_TIME, SCREEN_TEXT_AMPLITUDE, &analysis);
ReportTime reporttime(&screen, SCREEN_TEXT_DATEFILE, &rtclock, &analysis);

PushButtons buttons;
Menu menu;
Menu settings_menu;
Menu fishfinder_menu;
Menu logger_menu;
Blink blink(RECORD_LED_PIN);
Blink voiceled(VOICE_LED_PIN);

String prevname; // previous file name without increment
String lastname; // last recorded file name
int restarts = 0;
bool run = true;
int updownstate = 0;    // how to use up/down buttons
const int maxupdownstates = 3; // number of different usages for up/down buttons
char updownids[maxupdownstates][2] = {"V", "X", "Y"};
#ifdef DETECT_CLIPPING
char clippingids[2][2] = {"", "C"};
#endif

#ifdef DEBUG
#include <FreeStack.h>
  #define CHECK_MEMORY Serial.printf("Free Memory: %d\n", FreeStack());
#else
  #define CHECK_MEMORY 
#endif


void setupDataADC(int i) {
  aidata.clearChannels();
  aidata.setChannel(0, CHANNEL_FRONT);
  //aidata.setChannel(1, CHANNEL_BACK);
  aidata.configure(*ai_settings[i]);
  aidata.check();
}


void showDataADC(int id) {
  #ifdef LOGGER
  if (id > 2)
    setupDataADC(logger_settings.Mode);
  else
  #endif
    setupDataADC(settings.Mode);
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
  screen.fadeBacklightOff();
  screen.clear();
  screen.setTextArea(0, 0.0, 0.75, 1.0, 0.95);
  screen.setTitleFont(0);
  screen.setTextArea(1, 0.0, 0.0, 0.4, 0.7, true);
  screen.setSmallFont(1);
  screen.setTextArea(2, 0.4, 0.0, 1.0, 0.7, true);
  screen.setSmallFont(2);
  screen.writeText(0, SOFTWARE);
  screen.writeText(1, "rate:\nres.:\nspeed:\nADC0:\nADC1\nbuffer:");
  screen.writeText(2, msg);
  screen.fadeBacklightOn();
  buttons.waitPressAny();
  screen.fadeBacklightOff();
  screen.clearText();
}


void setupVoiceADC() {
  aidata.clearChannels();
  aidata.setChannel(0, CHANNEL_VOICE);
  aidata.configure(voice_settings);
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
  screen.clear();
  screen.setTextArea(SCREEN_TEXT_ACTION, 0.0, 0.9, 0.38, 1.0);
  screen.setTextArea(SCREEN_TEXT_DATEFILE, 0.4, 0.9, 1.0, 1.0);
#ifdef COMPUTE_SPECTRUM
  screen.setTextArea(SCREEN_TEXT_PEAKFREQ, 0.0, 0.77, 0.3, 0.87);
#endif
  screen.setTextArea(SCREEN_TEXT_FILETIME, 0.8, 0.77, 1.0, 0.87);
  screen.setTextArea(SCREEN_TEXT_UPDOWN, 0.95, 0.79, 1.0, 0.87, true);
  screen.setTextArea(SCREEN_TEXT_TIME, 0.0, 0.0, 0.25, 0.13);
  screen.setTextArea(SCREEN_TEXT_AMPLITUDE, 0.0, 0.63, 0.15, 0.76);
  screen.swapTextColors(SCREEN_TEXT_UPDOWN);
  screen.writeText(SCREEN_TEXT_UPDOWN, updownids[updownstate]);
#ifdef DETECT_CLIPPING
  screen.setTextArea(SCREEN_TEXT_CLIPPING, 0.89, 0.79, 0.94, 0.87, true);
  screen.swapTextColors(SCREEN_TEXT_CLIPPING);
  screen.writeText(SCREEN_TEXT_CLIPPING,
                   clippingids[clipping.feedbackEnabled()]);
#endif
  screen.setPlotAreas(1, 0.0, 0.0, 1.0, 0.82);
  screen.setBacklightOn();
}


void reactivateBaseScreen() {
    reporttime.enable();
    screen.clearText(SCREEN_TEXT_ACTION);
#ifdef DETECT_CLIPPING
    screen.writeText(SCREEN_TEXT_CLIPPING,
                     clippingids[clipping.feedbackEnabled()]);
#endif
    screen.writeText(SCREEN_TEXT_UPDOWN, updownids[updownstate]);
}


void diskFull() {
  Serial.println("SD card probably not inserted or full");
  Serial.println();
  screen.writeText(SCREEN_TEXT_ACTION, "!NO SD CARD!");
}


String makeFileName(const char *filename) {
  CHECK_MEMORY
  String name = filename;
  if (name.indexOf("DEV") >= 0)
    name.replace("DEV", device_settings.DeviceName);
  time_t t = now();
  name = rtclock.makeStr(name, t, true);
  if (name != prevname) {
    datafile.sdcard()->resetFileCounter();
    prevname = name;
  }
  name = datafile.sdcard()->incrementFileName(name);
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
  Serial.println(name);
  // all screen writing 210ms:
  String sname = name;
  int idx = sname.lastIndexOf('.');
  if (idx >= 0)
    sname.remove(idx);
  screen.writeText(SCREEN_TEXT_ACTION, "REC");
  screen.writeText(SCREEN_TEXT_DATEFILE, sname.c_str());
  blink.setSingle();
  blink.blinkSingle(0, 1000);
  return true;
}

#ifdef LOGGER
bool openNextFile(const String &name) {
  blink.clear();
  if (name.length() == 0)
    return false;
  String fname = name + ".wav";
  char dts[20];
  rtclock.dateTime(dts);
  if (! datafile.openWave(fname.c_str(), -1, dts)) {
    Serial.println();
    Serial.println("WARNING: failed to open file on SD card.");
    Serial.println("SD card probably not inserted or full -> halt");
    aidata.stop();
    while (1) {};
    return false;
  }
  datafile.write();
  Serial.println(fname);
  blink.setSingle();
  blink.blinkSingle(0, 1000);
  return true;
}
#endif


void startRecording() {
  reporttime.disable();
  String name = makeFileName(settings.FileName);
  openFile(name);
  SwapCounter = 0;
}


void stopRecording() {
  datafile.write();
  datafile.closeWave();
  blink.clear();
  screen.clearText(SCREEN_TEXT_ACTION);
  screen.clearText(SCREEN_TEXT_FILETIME);
  Serial.println("  stopped recording\n");
}


void startVoiceMessage() {
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
  #ifdef COMPUTE_SPECTRUM
  screen.clearText(SCREEN_TEXT_PEAKFREQ);
  #endif
  voiceled.setSingle();
  voiceled.blinkSingle(0, 1000);
  Serial.println("START VOICE MESSAGE");
  SwapCounter = 0;
}


void stopVoiceMessage() {
  voicefile.write();
  voicefile.closeWave();
  voiceled.clear();
  aidata.stop();
  screen.clearText(SCREEN_TEXT_ACTION);
  screen.clearText(SCREEN_TEXT_FILETIME);
  setupDataADC(settings.Mode);
  aidata.start();
  aidata.report();
  audio.play();
  analysis.start(ai_settings[settings.Mode]->analysisInterval(),
		 ai_settings[settings.Mode]->analysisWindow());
  Serial.println("STOP VOICE MESSAGE");
}


void toggleRecord(int id) {
  if (voicefile.isOpen()) // voice message in progress
    stopVoiceMessage();
  else if (datafile.isOpen())
    stopRecording();
  else if (! reporttime.enabled()) {
    if (lastname.length() > 0 &&
	buttons.button(id)->previousDuration() > 500) {
      sdcard.remove(lastname.c_str());
      lastname = "";
      screen.writeText(SCREEN_TEXT_ACTION, "DELETED");
    }
    else
      reactivateBaseScreen();
  }
  else if (buttons.button(id)->previousDuration() > 500)
    run = false;
  else
    startRecording();
  DateFileTime = 0;
}


void toggleVoiceMessage(int id) {
  if (reporttime.enabled()) {
#ifdef DETECT_CLIPPING
    if (buttons.button(id)->previousDuration() > 500) {
      clipping.toggleFeedback();
      screen.writeText(SCREEN_TEXT_CLIPPING,
                       clippingids[clipping.feedbackEnabled()]);
    }
    else {
#endif
      // change up/down switch usage:
      updownstate++;
      if (updownstate >= maxupdownstates)
        updownstate = 0;
      screen.writeText(SCREEN_TEXT_UPDOWN, updownids[updownstate]);
#ifdef DETECT_CLIPPING
    }
#endif
  }
  else {
    // voice message only as long as last file name is shown
    if (datafile.isOpen())       // recording in progress
      return;
    if (lastname.length() == 0)  // no recording yet
      return;
    if (voicefile.isOpen())
      stopVoiceMessage();
    else
      startVoiceMessage();
    DateFileTime = 0;
  }
}


void switchUp(int id) {
  switch (updownstate) {
  case 2:
    plotting.zoomAmplitudeIn();
    break;
  case 1:
    plotting.zoomTimeIn();
    break;
  default:
    audio.volumeUp();
  }
}


void switchDown(int id) {
  switch (updownstate) {
  case 2:
    plotting.zoomAmplitudeOut();
    break;
  case 1:
    plotting.zoomTimeOut();
    break;
  default:
    audio.volumeDown();
  }
}


void initButtons() {
  int select = buttons.add(RECORD_PIN, INPUT_PULLUP);
  int back = buttons.add(VOICE_PIN, INPUT_PULLUP);
  int up = buttons.add(UP_PIN, INPUT_PULLUP);
  int down = buttons.add(DOWN_PIN, INPUT_PULLUP);
  menu.setButtons(&buttons, up, down, select, back);
  menu.setDisplay(&screen);
}


void setupButtons() {
  buttons.set(buttons.id(RECORD_PIN), 0, toggleRecord);
  buttons.set(buttons.id(VOICE_PIN), 0, toggleVoiceMessage);
  buttons.set(buttons.id(UP_PIN), switchUp);
  buttons.set(buttons.id(DOWN_PIN), switchDown);
}


void setupStorage(const char *path) {
  prevname = "";
  lastname = "";
  datafile.sdcard()->rootDir();
  if (datafile.sdcard()->dataDir(path))
    Serial.printf("Save recorded data in folder \"%s\".\n\n", path);
  datafile.setWriteInterval();
  datafile.header().setSoftware(SOFTWARE);
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
        File mf = sdcard.openWrite(mfs);
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


#ifdef LOGGER
void loggerStoreData() {
  if (datafile.pending()) {
    ssize_t samples = datafile.write();
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
        datafile.closeWave();
        char mfs[20];
        sprintf(mfs, "error%d-%d.msg", restarts+1, -samples);
        File mf = sdcard.openWrite(mfs);
        mf.close();
      }
    }
    if (datafile.endWrite() || samples < 0) {
      datafile.close();  // file size was set by openWave()
      String name = makeFileName(logger_settings.FileName);
      if (samples < 0) {
        restarts++;
        if (restarts >= 5) {
          Serial.println("ERROR: Too many file errors -> halt.");
          aidata.stop();
          while (1) {};
        }
      }
      if (samples == -3) {
        aidata.start();
        datafile.start();
      }
      openNextFile(name);
    }
  }
}
#endif


void setupAudio() {
  //audio.setMixer(&AudioPlayBuffer::difference);
  audio.setMixer(&AudioPlayBuffer::assign);
  AudioMemory(AUDIO_BLOCKS);
  audio.setupAmp(AMPL_ENABLE_PIN);
  audio.setupVolume(0.1);
  audio.setLowpass(2);
  audio.addFeedback(0.3, 6*440.0, 0.17);
#ifdef COMPUTE_CORRELATIONS
  audio.addFeedback(0.2, 2*440.0, 0.2);
#endif
}


void setupAnalysis() {
#ifdef DETECT_CLIPPING
  clipping.setClipThreshold(0.9);   // make it configurable!
  clipping.setMuteThreshold(0.7);   // make it configurable!
#endif
#ifdef COMPUTE_CORRELATIONS
  correlation.disable();
#endif
#ifdef COMPUTE_SPECTRUM
  spectrum.setNFFT(4096);
  spectrum.setResolution(3.0);
  peakfreq.setFrequencyRange(SPECTRUM_FMIN, SPECTRUM_FMAX);
#endif
  plotting.setSkipping(4);
  plotting.setWindow(0.01);
  plotting.setAlignMax(0.5);        // align maximum in center of plot
  analysis.start(ai_settings[settings.Mode]->analysisInterval(),
		 ai_settings[settings.Mode]->analysisWindow());
}


void selectFishfinderMode(int id) {
  settings.Mode = id;
}


#ifdef LOGGER
void selectLoggerMode(int id) {
  logger_settings.Mode = id;
}
#endif


#ifdef MTP_RESPONDER
void runMTPResponder() {
  screen.clear();
  screen.setTextArea(0, 0.25, 0.7, 0.8, 0.8);
  screen.writeText(0, SOFTWARE);
  screen.setTextArea(1, 0.4, 0.4, 0.6, 0.6);
  screen.swapTextColors(1);
  screen.writeText(1, " MTP");
  screen.setTextArea(2, 0.3, 0.2, 0.7, 0.3);
  screen.writeText(2, "file transfer");
  screen.setBacklightOn();
  MTP.begin();
  MTP.addFilesystem(sdcard, "Fishfinder");
  int id = -1;
  while (1) {
    MTP.loop();
    buttons.update();
    id = buttons.pressedAny();
    if (id >= 0)
      break;
    yield();
  }
  buttons.waitReleased(id);
  screen.clear();
}
#endif


#ifdef SDFORMAT
void formatSDCard(int id=0) {
  screen.clear();
  screen.setTextArea(0, 0.25, 0.7, 0.8, 0.8);
  screen.writeText(0, SOFTWARE);
  screen.setTextArea(1, 0.22, 0.4, 0.78, 0.6);
  screen.swapTextColors(1);
  screen.writeText(1, " Format SD card");
  screen.setTextArea(2, 0.27, 0.2, 0.73, 0.3);
  screen.writeText(2, "in progress ...");
  screen.setBacklightOn();
  sdcard.format("fishfinder.cfg", true);
}
#endif


#ifdef LOGGER
void runLogger(int id=0) {
  screen.setBacklightOff();
  setupDataADC(logger_settings.Mode);
  setupStorage(logger_settings.Path);
  datafile.setMaxFileTime(logger_settings.FileTime);
  aidata.check();
  aidata.start();
  aidata.report();
  blink.switchOff();
  if (logger_settings.InitialDelay >= 2.0) {
    delay(1000);
    blink.setDouble();
    blink.delay(uint32_t(1000.0*logger_settings.InitialDelay) - 1000);
  }
  else
    delay(uint32_t(1000.0*logger_settings.InitialDelay));
  String name = makeFileName(logger_settings.FileName);
  if (name.length() == 0) {
    Serial.println("-> halt");
    aidata.stop();
    while (1) {};
  }
  datafile.start();
  openNextFile(name);
  while (1) {
    loggerStoreData();
    blink.update();
    yield();
  }
}
#endif


void runFishfinder(int id=0) {
  setupDataADC(settings.Mode);
  setupStorage(settings.Path);
  setupScreen();
  setupAudio();
  setupAnalysis();
  setupButtons();
  aidata.start();
  aidata.report();
  blink.switchOff();
  voiceled.switchOff();
  run = true;
  while (run) {
    buttons.update();
    storeData();
    analysis.update();        
    audio.update();
    blink.update();
    if ((DateFileTime > MAX_FILE_SHOWTIME) && ! reporttime.enabled())
      break;
    yield();
  }
  analysis.stop();
  audio.pause();
  aidata.stop();
  datafile.sdcard()->rootDir();
}


#ifdef STORE_SETUP
void resetSettings(int id) {
  int addr = 0;
  int mode = 0;
  EEPROM.get(addr, mode);
  if (mode != 1) {
    settings.Mode = 1;
    EEPROM.put(addr, settings.Mode);
  }
  addr += sizeof(settings.Mode);
#ifdef LOGGER
  EEPROM.get(addr, mode);
  if (mode != 1) {
    logger_settings.Mode = 1;
    EEPROM.put(addr, logger_settings.Mode);
  }
  addr += sizeof(logger_settings.Mode);
#endif
  char device_name[device_settings.MaxStr];
  EEPROM.get(addr, device_name);
  if (strcmp(device_name, DEVICE_NAME) != 0) {
    strcpy(device_settings.DeviceName, DEVICE_NAME);
    EEPROM.put(addr, device_settings.DeviceName);
  }
  addr += sizeof(device_settings.DeviceName);
  char title[50];
  strcpy(title, SOFTWARE);
  strcat(title, "     Device ");
  strcat(title, device_settings.DeviceName);
  menu.setTitle(title);
  screen.clear();
  screen.setTextArea(0, 0.25, 0.7, 0.8, 0.8);
  screen.writeText(0, SOFTWARE);
  screen.setTextArea(1, 0.17, 0.4, 0.83, 0.6);
  screen.swapTextColors(1);
  screen.writeText(1, " settings resetted! ");
  screen.setTextArea(2, 0.23, 0.1, 0.77, 0.2);
  screen.writeText(2, "press any button");
  screen.setBacklightOn();
  id = -1;
  while (1) {
    buttons.update();
    id = buttons.pressedAny();
    if (id >= 0)
      break;
    yield();
  }
  buttons.waitReleased(id);
  screen.clear();
}

void saveSettings(int id) {
  int addr = 0;
  int mode = 0;
  EEPROM.get(addr, mode);
  if (settings.Mode != mode)
    EEPROM.put(addr, settings.Mode);
  addr += sizeof(settings.Mode);
#ifdef LOGGER
  EEPROM.get(addr, mode);
  if (logger_settings.Mode != mode)
    EEPROM.put(addr, logger_settings.Mode);
  addr += sizeof(logger_settings.Mode);
#endif
  char device_name[device_settings.MaxStr];
  EEPROM.get(addr, device_name);
  if (strcmp(device_settings.DeviceName, device_name) != 0)
    EEPROM.put(addr, device_settings.DeviceName);
  addr += sizeof(device_settings.DeviceName);
  screen.clear();
  screen.setTextArea(0, 0.25, 0.7, 0.8, 0.8);
  screen.writeText(0, SOFTWARE);
  screen.setTextArea(1, 0.24, 0.4, 0.76, 0.6);
  screen.swapTextColors(1);
  screen.writeText(1, " settings saved! ");
  screen.setTextArea(2, 0.23, 0.1, 0.77, 0.2);
  screen.writeText(2, "press any button");
  screen.setBacklightOn();
  id = -1;
  while (1) {
    buttons.update();
    id = buttons.pressedAny();
    if (id >= 0)
      break;
    yield();
  }
  buttons.waitReleased(id);
  screen.clear();
}


void loadSettings() {
  int addr = 0;
  EEPROM.get(addr, settings.Mode);
  if (settings.Mode < 0 || settings.Mode >= MAX_SETTINGS)
    settings.Mode = 1;
  addr += sizeof(settings.Mode);
#ifdef LOGGER
  EEPROM.get(addr, logger_settings.Mode);
  if (logger_settings.Mode < 0 || logger_settings.Mode >= MAX_SETTINGS)
    logger_settings.Mode = 1;
  addr += sizeof(logger_settings.Mode);
#endif
  /*
  char device_name[device_settings.MaxStr];
  EEPROM.get(addr, device_name);
  strcpy(device_settings.DeviceName, device_name);
  addr += sizeof(device_settings.DeviceName);
  */
}
#endif


void initMenu() {
#ifdef ADC_SETUP
  fishfinder_menu.setTitle("Fishfinder recording mode");
#ifdef LOGGER
  logger_menu.setTitle("Logger recording mode");
#endif
  for (int k=0; k<MAX_SETTINGS; k++) {
    if (ai_settings[k]->analysisWindow() > 0)
      fishfinder_menu.addRadioButton(ai_settings[k]->name(),
				     k==settings.Mode);
#ifdef LOGGER
    logger_menu.addRadioButton(ai_settings[k]->name(),
			       k==logger_settings.Mode);
#endif
  }
  fishfinder_menu.setCheckedAction(selectFishfinderMode);
  fishfinder_menu.setCheckedReturns();
#ifdef LOGGER
  logger_menu.setCheckedAction(selectLoggerMode);
  logger_menu.setCheckedReturns();
#endif
  settings_menu.addMenu("Setup fishfinder", fishfinder_menu);
#ifdef LOGGER
  settings_menu.addMenu("Setup logger", logger_menu);
#endif
  settings_menu.addAction("Show fishfinder settings", showDataADC, 2);
#ifdef LOGGER
  settings_menu.addAction("Show logger settings", showDataADC, 3);
#endif
#ifdef STORE_SETUP
  settings_menu.addAction("Save settings", saveSettings);
  settings_menu.addAction("Reset settings", resetSettings);
#endif
#endif
  char title[50];
  strcpy(title, SOFTWARE);
  strcat(title, "     Device ");
  strcat(title, device_settings.DeviceName);
  menu.setTitle(title);
  menu.addAction("Run as fishfinder", runFishfinder, 0);
#ifdef LOGGER
  menu.addAction("Run as logger", runLogger, 1);
#endif
#ifdef ADC_SETUP
  menu.addMenu("Settings", settings_menu);
#else
  menu.addAction("Show fishfinder settings", showDataADC, 2);
#ifdef LOGGER
  menu.addAction("Show logger settings", showDataADC, 3);
#endif
#endif
#ifdef SDFORMAT
  menu.addAction("Format SD card", formatSDCard, 4);
#endif
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
  initScreen(screen);
  loadSettings();
  sdcard.begin();
  config.setConfigFile("fishfinder.cfg");
  config.configure(sdcard);
  initMenu();
  initButtons();
  DateFileTime = 0;
  screen.setBacklightOn();
  if (menu.nActions() > 1) {
    while (1) {
#ifdef MTP_RESPONDER
      if (usb_configuration)
	runMTPResponder();
#endif
      menu.exec();
    }
  }
  runFishfinder();
}


void loop() {
}
