#include <DeviceSettings.h>


DeviceSettings::DeviceSettings(const char *devicename) :
  Configurable("Device"),
  DeviceName(this, "DeviceName", devicename) {
}
