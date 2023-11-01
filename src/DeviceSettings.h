/*
  DeviceSettings - configurable device name.
  Created by Jan Benda, January 7th, 2023.
*/

#ifndef DeviceSettings_h
#define DeviceSettings_h


#include <Configurable.h>


class DeviceSettings : public Configurable {

public:

  static const int MaxStr = 16;

  DeviceSettings(const char *devicename="1");

  const char *deviceName() const { return DeviceName.value(); };
  void setDeviceName(const char *devicename) { DeviceName.setValue(devicename); };
  
  
protected:

  StringParameter<MaxStr> DeviceName;
  
};

#endif
