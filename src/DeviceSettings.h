/*
  DeviceSettings - configurable device name.
  Created by Jan Benda, January 7th, 2023.
*/

#ifndef DeviceSettings_h
#define DeviceSettings_h


#include <Configurable.h>


class DeviceSettings : public Configurable {

public:

  DeviceSettings(const char *device_name="1");
  
  virtual void configure(const char *key, const char *val);

  static const size_t MaxStr = 10;
  char DeviceName[MaxStr];
};

#endif
