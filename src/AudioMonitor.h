/*
  AudioMonitor - 
  Created by Jan Benda, May 25, 2022.
*/

#ifndef AudioMonitor_h
#define AudioMonitor_h


#include <Arduino.h>
#include <Audio.h>
#include <Bounce2.h>


class AudioMonitor {
  
 public:
  
  AudioMonitor(AudioMixer4 &mixer, float volume=0.1);

  void setup(int volume_up_pin, int volume_down_pin, int mode=INPUT_PULLUP);

  void update();
  

 protected:

  void volumeUp();
  void volumeDown();

  AudioMixer4 *Mixer;
  float Volume;
  Button VolumeUpButton;
  Button VolumeDownButton;

  

};


#endif
