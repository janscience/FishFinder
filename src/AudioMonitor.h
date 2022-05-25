/*
  AudioMonitor - Play recorded data on speaker.
  Created by Jan Benda, May 25, 2022.
*/

#ifndef AudioMonitor_h
#define AudioMonitor_h


#include <Arduino.h>
#include <Audio.h>
#include <Bounce2.h>
#include <DataWorker.h>
#include <AudioPlayBuffer.h>


class AudioMonitor {
  
 public:
  
  AudioMonitor(DataWorker &data, AudioStream &speaker);

  void setup(int amplifier_pin=-1, float volume=0.8, int volume_up_pin=-1,
	     int volume_down_pin=-1, int mode=INPUT_PULLUP);

  void update();
  

 protected:

  void volumeUp();
  void volumeDown();

  AudioPlayBuffer Data;
  AudioMixer4 *Mixer;
  AudioConnection *AC1;
  AudioConnection *ACO;
  float Volume;
  bool VolumeButtons;
  Button VolumeUpButton;
  Button VolumeDownButton;

  

};


#endif
