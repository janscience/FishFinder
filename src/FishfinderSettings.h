/*
  FishfinderSettings - configurable settings for fishfinder (file name, path, etc.).
  Created by Jan Benda, December 17th, 2022.
*/

#ifndef FishfinderSettings_h
#define FishfinderSettings_h


#include <Configurable.h>


class FishfinderSettings : public Configurable {

public:

  FishfinderSettings(const char *path="recordings",
		     const char *filename="SDATELNUM.wav",
		     int mode=0);

  static int modeEnum(const char *mode);
  
  virtual void configure(const char *key, const char *val);

  static const size_t MaxStr = 99;
  char Path[MaxStr + 1];
  char FileName[MaxStr + 1];
  int Mode;
};

#endif
