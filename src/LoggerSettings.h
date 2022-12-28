/*
  Settings - common configurable settings (file name, path, etc.).
  Created by Jan Benda, July 8th, 2021.
*/

#ifndef LoggerSettings_h
#define LoggerSettings_h


#include <Arduino.h>
#include <FishfinderSettings.h>


class LoggerSettings : public FishfinderSettings {

public:

  LoggerSettings(const char *path="recordings",
		 const char *filename="SDATELNUM.wav",
		 int mode=0,
		 float filetime=10.0,
		 float initialdelay=0.0);
  
  virtual void configure(const char *key, const char *val);

  float FileTime;
  float InitialDelay;
};

#endif
