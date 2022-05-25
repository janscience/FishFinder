#include "AudioMonitor.h"


AudioMonitor::AudioMonitor(DataWorker &data, AudioStream &speaker) :
  Data(data),
  Volume(0.8),
  VolumeButtons(false) {
  Mixer = new AudioMixer4;
  AC1 = new AudioConnection(Data, 0, *Mixer, 0);
  ACO = new AudioConnection(*Mixer, 0, speaker, 0);
}


void AudioMonitor::setup(int amplifier_pin, float volume,
			 int volume_up_pin, int volume_down_pin, int mode) {
  AudioMemory(16);
  Volume = volume;
  Mixer->gain(0, Volume);
  if (volume_up_pin >= 0 && volume_down_pin >= 0) {
    VolumeUpButton.attach(volume_up_pin, mode);
    VolumeUpButton.setPressedState(mode==INPUT_PULLUP?LOW:HIGH);
    VolumeUpButton.interval(20);
    VolumeDownButton.attach(volume_down_pin, mode);
    VolumeDownButton.setPressedState(mode==INPUT_PULLUP?LOW:HIGH);
    VolumeDownButton.interval(20);
    VolumeButtons = true;
  }
  if ( amplifier_pin >= 0 ) {
    pinMode(amplifier_pin, OUTPUT);
    digitalWrite(amplifier_pin, HIGH); // turn on the amplifier
    delay(10);                         // allow time to wake up
  }
}


void AudioMonitor::volumeUp() {
  Volume *= 1.414213;
  if (Volume > 0.8)
    Volume = 0.8;
  Mixer->gain(0, Volume);
}


void AudioMonitor::volumeDown() {
  Volume /= 1.414213;
  if (Volume < 0.00625)
    Volume = 0.00625;
  Mixer->gain(0, Volume);
}


void AudioMonitor::update() {
  if (!VolumeButtons)
    return;
  VolumeUpButton.update();
  VolumeDownButton.update();
  if (VolumeUpButton.pressed())
    volumeUp();
  if (VolumeDownButton.pressed())
    volumeDown();
}




