/*
  Settings - common configurable settings (file name, path, etc.).
  Created by Jan Benda, July 8th, 2021.
*/

#ifndef LoggerSettings_h
#define LoggerSettings_h


#include <Settings.h>


class LoggerSettings : public Settings {

public:

  LoggerSettings(const char *path="recordings",
		 const char *filename="SDATELNUM.wav",
		 float filetime=10.0,
		 float initialdelay=0.0);
  
  virtual void configure(const char *key, const char *val);

  float FileTime;
  float InitialDelay;
};

#endif
