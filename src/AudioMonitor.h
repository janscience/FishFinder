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

  void addFeedback(float gain, float freq, float duration);
  void setFeedbackInterval(uint interval, uint8_t soundidx=0);

  void update();
  

 protected:

  void volumeUp();
  void volumeDown();
  void setVolume();

  AudioPlayBuffer Data;
  AudioPlayMemory *Sound[3];
  AudioMixer4 *Mixer;
  AudioConnection *ACI;
  AudioConnection *ACO;
  AudioConnection *ACS[3];
  int NSounds;
  float Volume;
  float Gains[4];
  bool VolumeButtons;
  Button VolumeUpButton;
  Button VolumeDownButton;
  int16_t *Beep[3];
  uint BeepInterval[3];
  elapsedMillis BeepTime[3];

  

};


#endif
