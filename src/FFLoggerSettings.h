/*
  FFLoggerSettings - common configurable settings (file name, path, etc.).
  Created by Jan Benda, July 8th, 2021.
*/

#ifndef FFLoggerSettings_h
#define FFLoggerSettings_h


#include <Arduino.h>
#include <FishfinderSettings.h>


class FFLoggerSettings : public FishfinderSettings {

public:

  FFLoggerSettings(const char *path="recordings",
		   const char *filename="SDATELNUM.wav",
		   int mode=0, float filetime=10.0,
		   float initialdelay=0.0);

  float fileTime() const { return FileTime.value(); };
  float initialDelay() const { return InitialDelay.value(); };

  
protected:

  NumberParameter<float> FileTime;
  NumberParameter<float> InitialDelay;
  
};

#endif
