#include "AudioMonitor.h"


AudioMonitor::AudioMonitor(AudioMixer4 &mixer, float volume) :
  Mixer(&mixer),
  Volume(volume) {
}


void AudioMonitor::setupVolume(int volume_up_pin, int volume_down_pin, int mode) {
  VolumeUpButton.attach(volume_up_pin, mode);
  VolumeUpButton.setPressedState(mode==INPUT_PULLUP?LOW:HIGH);
  VolumeUpButton.interval(20);
  VolumeDownButton.attach(volume_down_pin, mode);
  VolumeDownButton.setPressedState(mode==INPUT_PULLUP?LOW:HIGH);
  VolumeDownButton.interval(20);
  Mixer->gain(0, Volume);
}


void AudioMonitor::setupAmplifier(int amplifier_pin) {
  AudioMemory(16);
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
  VolumeUpButton.update();
  VolumeDownButton.update();
  if (VolumeUpButton.pressed())
    volumeUp();
  if (VolumeDownButton.pressed())
    volumeDown();
}




