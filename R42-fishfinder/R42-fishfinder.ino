// Features: ------------------------------------------------------------------

// Compute spectrum and detect peak frequency:
#define COMPUTE_SPECTRUM

#include <FishfinderBanner.h>
#include <Wire.h>
#include <ControlPCM186x.h>
#include <InputTDM.h>
#include <AudioMonitor.h>
#include <SDCard.h>
#include <SDWriter.h>
#include <DeviceID.h> 
#include <Display.h>
#include <ST7789_t3.h>
#include <fonts/FreeSans12pt7b.h>
#include <AnalysisChain.h>
#include <Plotting.h>
#ifdef COMPUTE_SPECTRUM
#include <Spectrum.h>
#include <ReportPeakFreq.h>
#endif
#include <MicroConfig.h>
#include <Settings.h>
#include <InputTDMSettings.h>

// Default settings: ----------------------------------------------------------
// (may be overwritten by config file fishgrid.cfg)
#define NCHANNELS        2        // number of channels (even, from 2 to 16)
#define SAMPLING_RATE    48000    // samples per second and channel in Hertz
#define PREGAIN          10.0     // gain factor of preamplifier
#define GAIN             0.0      // dB

#define DEVICEID         1                  // may be used for naming pathes and files
#define PATH             "ffID1-SDATE"   // folder where to store the recordings, may include ID, IDA, DATE, SDATE, TIME, STIME, DATETIME, SDATETIME, NUM
#define FILENAME         "ffID1-SDATENNUM3.wav"  // may include ID, IDA, DATE, SDATE, TIME, STIME, DATETIME, SDATETIME, NUM, ANUM, COUNT

#define AUDIO_BLOCKS     4

#define ANALYSIS_INTERVAL  0.2 // seconds
#define ANALYSIS_WINDOW    0.2 // seconds

#ifdef COMPUTE_SPECTRUM
#define SPECTRUM_FMIN  70.0   // Hz
#define SPECTRUM_FMAX  2500.0 // Hz, 1.0e9 or larger: take all upto Nyquist frequency.
#endif


// Pin assignment: ------------------------------------------------------------

#define AMPL_ENABLE_PIN   6  // pin for enabling audio amplifier


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

AudioOutputI2S speaker;
AudioMonitor audio(aidata, speaker);

Display screen;
ST7789_t3 tft(TFT_CS_PIN, TFT_DC_PIN, TFT_MOSI_PIN,
              TFT_SCK_PIN, TFT_RST_PIN);

AnalysisChain analysis(aidata);
#ifdef COMPUTE_SPECTRUM
Spectrum spectrum(0, &analysis);
ReportPeakFreq peakfreq(&spectrum, &screen, SCREEN_TEXT_PEAKFREQ, &analysis);
#endif
Plotting plotting(0, 0, &screen, 0, SCREEN_TEXT_TIME, SCREEN_TEXT_AMPLITUDE,
                  &analysis);

DeviceID deviceid(DEVICEID);
SDCard sdcard;
SDWriter datafile(sdcard, aidata);

Config config("fishfinder.cfg", &sdcard);
Settings settings(config, PATH, DEVICEID, FILENAME);
InputTDMSettings aisettings(config, SAMPLING_RATE, NCHANNELS, GAIN, PREGAIN);

String prevname; // previous file name without increment
String lastname; // last recorded file name
int restarts = 0;
int updownstate = 0;    // how to use up/down buttons
const int maxupdownstates = 4; // number of different usages for up/down buttons
char updownids[maxupdownstates][2] = {"V", "G", "X", "Y"};


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
  audio.setupVolume(0.1);
  audio.setLowpass(2);
  audio.addFeedback(0.3, 6*440.0, 0.1);
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
  analysis.start(ANALYSIS_INTERVAL, ANALYSIS_WINDOW);
}


void setup() {
  screen.setBacklightPin(TFT_BL_PIN);
  aisettings.setRateSelection(ControlPCM186x::SamplingRates,
                              ControlPCM186x::MaxSamplingRates);
  aisettings.enable("Pregain");
  Serial.begin(9600);
  while (!Serial && millis() < 200) {};
  printFishfinderBanner();
  sdcard.begin();
  //config.load();
  //if (Serial)
  //  config.execute(Serial, 10000);
  config.report();
  Serial.println();
  deviceid.setID(settings.deviceID());
  setupAIData();
  initScreen();
  screen.setBacklightOn();
  setupScreen();
  setupAudio();
  setupAnalysis();
  aidata.begin();
  aidata.start();
  aidata.report();
  String name = "recNUM.wav";
  name = datafile.sdcard()->incrementFileName(name);
  datafile.openWave(name.c_str(), -1, "2025-08-23T22:23:14");
  datafile.setMaxFileSamples(0);
  datafile.start();
  screen.writeText(SCREEN_TEXT_ACTION, "REC");
}


void loop() {
  analysis.update();
  audio.update();
  if (millis() > 15000) {
    datafile.closeWave();
    screen.clearText(SCREEN_TEXT_ACTION);
  }
  else if (datafile.pending())
    datafile.write();
}
