#include <FishfinderBanner.h>
#include <Wire.h>
#include <ControlPCM186x.h>
#include <InputTDM.h>
#include <InputADC.h>
#include <AudioMonitor.h>
#include <SDCard.h>
#include <SDWriter.h>
#include <DeviceID.h> 
#include <Display.h>
#include <ST7789_t3.h>
#include <fonts/FreeSans12pt7b.h>
#include <AnalysisChain.h>
#include <Plotting.h>
#include <Spectrum.h>
#include <ReportPeakFreq.h>
#include <ReportTime.h>
#include <RTClock.h>
#include <PushButtons.h>
#include <MicroConfig.h>
#include <Settings.h>
#include <InputTDMSettings.h>
#include <InputADCSettings.h>
#include <RTClockMenu.h>
#include <SDCardMenu.h>
#include <DiagnosticMenu.h>

// Default settings: ----------------------------------------------------------
// (may be overwritten by config file fishgrid.cfg)
#define NCHANNELS            2    // number of channels (even, from 2 to 16)
#define SAMPLING_RATE    96000    // samples per second and channel in Hertz
#define PREGAIN           10.0    // gain factor of preamplifier
#define GAIN              20.0    // dB

#define DEVICEID         1        // may be used for naming pathes and files
#define PATH             "ffID1-SDATE"   // folder where to store the recordings, may include ID, IDA, DATE, SDATE, TIME, STIME, DATETIME, SDATETIME, NUM
#define FILENAME         "ffID1-SDATENNUM2.wav"  // may include ID, IDA, DATE, SDATE, TIME, STIME, DATETIME, SDATETIME, NUM, ANUM, COUNT

#define AUDIO_BLOCKS     4

#define VOICE_SAMPLING_RATE 22050 // samples per second and channel in Hertz, 22.05kHz, 44.1kHz 96kHz, or 192kHz
#define VOICE_AVERAGING         4 // number of averages per sample: 0, 4, 8, 16, 32
#define VOICE_CONVERSION    ADC_CONVERSION_SPEED::HIGH_SPEED
#define VOICE_SAMPLING      ADC_SAMPLING_SPEED::HIGH_SPEED

#define MAX_TEXT_SWAP   20

#define ANALYSIS_INTERVAL  0.25     // seconds
#define ANALYSIS_WINDOW    0.2      // seconds

#define SPECTRUM_FMIN    30.0       // Hz
#define SPECTRUM_FMAX  2500.0       // Hz, 1.0e9 or larger: take all upto Nyquist frequency.


// Pin assignment: ------------------------------------------------------------

#define CHANNEL_VOICE   A17  // input pin for voice message

#define AMPL_ENABLE_PIN   6  // pin for enabling audio amplifier

#define DOWN_PIN         28  // pin for push button for decreasing audio volume/zoom
#define UP_PIN           29  // pin for push button for increasing audio volume/zoom
#define RECORD_PIN       30  // pin for push button starting and stopping a recording
#define VOICE_PIN        31  // pin for push button starting and stopping a voice message


// TFT display: ---------------------------------------------------------------

#define TFT_ROTATION      3

#define TFT_SCK_PIN      13   // SPI bus
#define TFT_CS_PIN       10  
#define TFT_MOSI_PIN     11   // SPI bus
#define TFT_RST_PIN       8
#define TFT_DC_PIN       14
#define TFT_BL_PIN       15   // backlight PWM

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

#define SOFTWARE "R42-FishFinder V2.0"

EXT_DATA_BUFFER(AIBuffer, NAIBuffer, 16*512*256)
InputTDM aidata(AIBuffer, NAIBuffer);
ControlPCM186x pcm(Wire1, PCM186x_I2C_ADDR1, InputTDM::TDM2);
ControlPCM186x pcm2(Wire1, PCM186x_I2C_ADDR2, InputTDM::TDM2);

DATA_BUFFER(VoiceBuffer, NVoiceBuffer, 32*512);
InputADC voicedata(VoiceBuffer, NVoiceBuffer);

AudioOutputI2S speaker;
AudioMonitor audio(aidata, speaker);

Display screen;
ST7789_t3 tft(TFT_CS_PIN, TFT_DC_PIN, TFT_MOSI_PIN,
              TFT_SCK_PIN, TFT_RST_PIN);

DeviceID deviceid(DEVICEID);
SDCard sdcard;
SDWriter datafile(sdcard, aidata);
SDWriter voicefile(sdcard, voicedata);

RTClock rtclock;
PushButtons buttons;

AnalysisChain analysis(aidata);
Spectrum spectrum(0, &analysis);
ReportPeakFreq peakfreq(&spectrum, &screen, SCREEN_TEXT_PEAKFREQ, &analysis);
Plotting plotting(0, 0, &screen, 0, SCREEN_TEXT_TIME, SCREEN_TEXT_AMPLITUDE,
                  &analysis);
ReportTime reporttime(&screen, SCREEN_TEXT_DATEFILE, &rtclock, &analysis);

Config config("fishfinder.cfg", &sdcard);
Settings settings(config, PATH, DEVICEID, FILENAME);
InputTDMSettings aisettings(config, SAMPLING_RATE, NCHANNELS, GAIN, PREGAIN);
InputADCSettings voicesettings("Voice ADC", VOICE_SAMPLING_RATE, 12,
		 	       VOICE_AVERAGING, VOICE_CONVERSION,
			       VOICE_SAMPLING, ADC_REFERENCE::REF_3V3);
RTClockMenu rtclock_menu(config, rtclock);
ConfigurationMenu configuration_menu(config, sdcard);
SDCardMenu sdcard_menu(config, sdcard);
DiagnosticMenu diagnostic_menu(config, sdcard, &pcm, &rtclock);
HelpAction help_act(config, "Help");

int SwapCounter;
elapsedMillis DateFileTime;

String prevname; // previous file name without increment
String lastname; // last recorded file name
int restarts = 0;

int updownstate = 0;    // how to use up/down buttons
const int maxupdownstates = 4; // number of different usages for up/down buttons
char updownids[maxupdownstates][2] = {"V", "G", "X", "Y"};
#ifdef DETECT_CLIPPING
char clippingids[2][2] = {"", "C"};
#endif

float gain = 20.0;
float max_gain = 40.0;
float gain_step = 5.0;


void initScreen() {
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
  screen.setTextArea(SCREEN_TEXT_DATEFILE, 0.38, 0.9, 1.0, 1.0);
  screen.setTextArea(SCREEN_TEXT_PEAKFREQ, 0.0, 0.77, 0.3, 0.87);
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


void setupAIData() {
  Wire1.begin();
  aidata.clearChannels();
  aisettings.configure(&aidata);
  aidata.setSwapLR();
  Serial.printf("Setup PCM186x on TDM %d: ", pcm.TDMBus());
  pcm.begin();
  bool r = pcm.setMicBias(false, true);
  if (!r) {
    Serial.println("not available");
    return;
  }
  pcm.setRate(aidata, aisettings.rate());
  pcm.setupTDM(aidata, ControlPCM186x::CH2L, ControlPCM186x::CH2R,
               0, ControlPCM186x::INVERTED);
  Serial.println("configured for 2 channels");
  pcm.setSmoothGainChange(false);
  pcm.setGainDecibel(aidata, aisettings.gainDecibel());
  pcm.setFilters(ControlPCM186x::FIR, false);
  Serial.println();
  Serial.printf("Setup PCM186x on TDM %d: ", pcm2.TDMBus());
  pcm2.setupTDM(ControlPCM186x::CH2L, ControlPCM186x::CH2R, 1);
  pcm2.powerdown();
  Serial.println("powered down");
}


void setupAudio() {
  //audio.setMixer(&AudioPlayBuffer::difference);
  audio.setMixer(&AudioPlayBuffer::assign);
  AudioMemory(AUDIO_BLOCKS);
  audio.setupAmp(AMPL_ENABLE_PIN);
  audio.setupVolume(0.5);
  audio.setLowpass(2);
  audio.addFeedback(0.3, 6*440.0, 0.1);
}


void setupAnalysis() {
#ifdef DETECT_CLIPPING
  clipping.setClipThreshold(0.9);   // make it configurable!
  clipping.setMuteThreshold(0.7);   // make it configurable!
#endif
  spectrum.setNFFT(4096);
  spectrum.setResolution(3.0);
  peakfreq.setFrequencyRange(SPECTRUM_FMIN, SPECTRUM_FMAX);
  //plotting.setSkipping(4);
  plotting.setWindow(0.01);
  plotting.setAlignMax(0.5);        // align maximum in center of plot
  analysis.start(ANALYSIS_INTERVAL, ANALYSIS_WINDOW);
}


void startRecording() {
  reporttime.disable();
  String name = makeFileName(settings.fileName());
  openFile(name);
  SwapCounter = 0;
}


void stopRecording() {
  datafile.write();
  datafile.closeWave();
  screen.clearText(SCREEN_TEXT_ACTION);
  screen.clearText(SCREEN_TEXT_FILETIME);
  Serial.println("  stopped recording\n");
}


void setupVoiceADC() {
  voicedata.clearChannels();
  voicedata.setChannel(0, CHANNEL_VOICE);
  voicesettings.configure(&voicedata);
  voicedata.check();
}


void startVoiceMessage() {
  audio.pause();
  setupVoiceADC();
  voicedata.start();
  voicedata.report();
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
  Serial.println("START VOICE MESSAGE");
  SwapCounter = 0;
}


void stopVoiceMessage() {
  voicefile.write();
  voicefile.closeWave();
  voicedata.stop();
  screen.clearText(SCREEN_TEXT_ACTION);
  screen.clearText(SCREEN_TEXT_FILETIME);
  audio.play();
  Serial.println("STOP VOICE MESSAGE");
}


void toggleRecord(int id) {
  Serial.println("TOGGLE RECORD");
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
  else
    startRecording();
  DateFileTime = 0;
}


void toggleVoiceMessage(int id) {
  Serial.println("TOGGLE VOICE");
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
    /* else
      startVoiceMessage(); */
    DateFileTime = 0;
  }
}


void setGain() {
  audio.pause();
  pcm.setGainDecibel(aidata, gain);
  Serial.printf("Set gain to %.0fdB\n", gain);
  audio.play();
}


void switchUp(int id) {
  Serial.println("SWITCH UP");
  if (voicefile.isOpen())
    return;
  switch (updownids[updownstate][0]) {
  case 'G':
    if (!datafile.isOpen() && gain < max_gain) {
      gain += gain_step;
      if ( gain > max_gain)
        gain = max_gain;
      setGain();
    }
    break;
  case 'Y':
    plotting.zoomAmplitudeIn();
    break;
  case 'X':
    plotting.zoomTimeIn();
    break;
  default:
    audio.volumeUp();
  }
}


void switchDown(int id) {
  Serial.println("SWITCH DOWN");
  if (voicefile.isOpen())
    return;
  switch (updownids[updownstate][0]) {
  case 'G':
    if (!datafile.isOpen() && gain > 0.0) {
      gain -= gain_step;
      if (gain < 0.0)
        gain = 0.0;
      setGain();
    }
    break;
  case 'Y':
    plotting.zoomAmplitudeOut();
    break;
  case 'X':
    plotting.zoomTimeOut();
    break;
  default:
    audio.volumeDown();
  }
}


void setupButtons() {
  int record = buttons.add(RECORD_PIN, INPUT_PULLUP);
  int voice = buttons.add(VOICE_PIN, INPUT_PULLUP);
  int up = buttons.add(UP_PIN, INPUT_PULLUP);
  int down = buttons.add(DOWN_PIN, INPUT_PULLUP);
  buttons.set(record, 0, toggleRecord);
  buttons.set(voice, 0, toggleVoiceMessage);
  buttons.set(up, switchUp);
  buttons.set(down, switchDown);
}


void diskFull() {
  Serial.println("SD card probably not inserted or full");
  Serial.println();
  screen.writeText(SCREEN_TEXT_ACTION, "!NO SD CARD!");
}


String makeFileName(const char *filename) {
  String name = SDCard::preparePath(filename, settings.deviceID());
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
  return true;
}


void setupStorage(const char *path) {
  prevname = "";
  lastname = "";
  datafile.sdcard()->rootDir();
  String path_name = makeFileName(path);
  if (datafile.sdcard()->dataDir(path_name.c_str()))
    Serial.printf("Save recorded data in folder \"%s\".\n\n",
    path_name.c_str());
  datafile.setWriteInterval(0.01);
  datafile.header().setSoftware(SOFTWARE);
  datafile.header().setCPUSpeed();
}


void storeData() {
  bool swap = false;
  if (datafile.pending()) {
    ssize_t samples = datafile.write();
    if (samples <= 0) {
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
        char mfs[30];
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


void setup() {
  screen.setBacklightPin(TFT_BL_PIN);
  settings.disable("FileTime");
  aisettings.setRateSelection(ControlPCM186x::SamplingRates,
                              ControlPCM186x::MaxSamplingRates);
  aisettings.enable("Pregain");
  Serial.begin(9600);
  while (!Serial && millis() < 200) {};
  printFishfinderBanner();
  rtclock.check();
  rtclock.report();
  sdcard.begin();
  config.load();
  if (Serial)
    config.execute();
  config.report();
  Serial.println();
  deviceid.setID(settings.deviceID());
  setupAIData();
  initScreen();
  screen.setBacklightOn();
  setupStorage(settings.path());
  setupScreen();
  setupAudio();
  setupAnalysis();
  setupButtons();
  aidata.begin();
  aidata.start();
  aidata.report();
}


void loop() {
  buttons.update();
  storeData();
  analysis.update();
  audio.update();
  if (datafile.pending())
    datafile.write();
}
