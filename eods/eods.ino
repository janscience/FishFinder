#include <TestSignals.h>
#include <Blink.h>
#include <PushButtons.h>

// ----- settings: -----------------------------------------------------------

float default_rate = 100000.0;
float ampl = 0.5;

float lepto_freq = 630.0;
float lepto_ampls[] = {0.170, 0.080, 0.014, 0.013, -1.0};
float lepto_phases[] = {0.936, 0.507, 0.554, 0.488, -1.0};

float rostratus_freq = 942.0;
float rostratus_ampls[] = {0.678, 0.098, 0.114, 0.062, 0.036, 0.015, -1.0};
float rostratus_phases[] = {0.394, 0.693, 0.893, 0.496, 0.980, 0.112, -1.0};

float eigen_freq = 360.0;
float eigen_ampls[] = {0.23, 0.06, 0.02, 0.01, 0.008, -1.0};
float eigen_phases[] = {0.784, 0.825, 0.594, 0.360, 0.267, -1.0};

// ----- pins: ---------------------------------------------------------------

#define DAC_PIN         A21

#define FREQ_UP_PIN     1   // switch for increasing frequency
#define FREQ_DOWN_PIN   2   // switch for decreasing frequency
#define AMPL_UP_PIN     3   // switch for increasing amplitude
#define AMPL_DOWN_PIN   4   // switch for decreasing amplitude
#define WAVEFORM_PIN    5   // switch for changing EOD waveform

// ---------------------------------------------------------------------------

Waveform wave;
Blink blink(LED_BUILTIN);
PushButtons buttons;

float rate = default_rate;

int fish = 0;
const int max_fish = 3;

float freqs[max_fish] = {lepto_freq, rostratus_freq, eigen_freq};
float *ampls[max_fish] = {lepto_ampls, rostratus_ampls, eigen_ampls};
float *phases[max_fish] = {lepto_phases, rostratus_phases, eigen_phases};


void frequency_up(int id) {
  rate *= pow(2.0, 1.0/12);
  wave.restart(rate);
}


void frequency_down(int id) {
  rate /= pow(2.0, 1.0/12);
  wave.restart(rate);
}


void amplitude_up(int id) {
  ampl *= 1.1;
  if (ampl > 1.0) 
    ampl = 1.0;
  wave.setAmplitude(ampl);
}


void amplitude_down(int id) {
  ampl /= 1.1;
  if (ampl < 0.0001) 
    ampl = 0.0001;
  wave.setAmplitude(ampl);
}


void next_fish(int id) {
  fish++;
  if (fish >= max_fish)
    fish = 0;
  wave.stop();
  start_fish();
}


void start_fish() {
  rate = default_rate;
  wave.setup(default_rate);
  wave.setHarmonics(ampls[fish], phases[fish]);
  wave.start(DAC_PIN, freqs[fish], 1.0);
  wave.setAmplitude(ampl);
}


void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 100) {};
  blink.setSingle();
  buttons.add(FREQ_UP_PIN, INPUT_PULLUP, frequency_up);
  buttons.add(FREQ_DOWN_PIN, INPUT_PULLUP, frequency_down);
  buttons.add(AMPL_UP_PIN, INPUT_PULLUP, amplitude_up);
  buttons.add(AMPL_DOWN_PIN, INPUT_PULLUP, amplitude_down);
  buttons.add(WAVEFORM_PIN, INPUT_PULLUP, next_fish);
  fish = 0;
  start_fish();
}


void loop() {
  buttons.update();
  blink.update();
}
