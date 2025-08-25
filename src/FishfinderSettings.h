/*
  FishfinderSettings - configurable settings for fishfinder (file name, path, etc.).
  Created by Jan Benda, December 17th, 2022.
*/

#ifndef FishfinderSettings_h
#define FishfinderSettings_h


#include <Parameter.h>
#include <Menu.h>


class FishfinderSettings : public Menu {

public:

  static const size_t MaxStr = 128;

  static const int MaxModes = 4;

  static const char *ModeStrings[MaxModes];
  
  static const int ModeEnums[MaxModes];

  FishfinderSettings(const char *path="recordings",
		     const char *filename="SDATELNUM.wav",
		     int mode=0);

  const char *path() const { return Path.value(); };
  const char *fileName() const { return FileName.value(); };
  int mode() const { return Mode.value(); };
  void setMode(int mode) { Mode.setValue(mode); };

  
protected:

  StringParameter<MaxStr> Path;
  StringParameter<MaxStr> FileName;
  EnumParameter<int> Mode;
  
};

#endif
