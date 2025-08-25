#include <DeviceSettings.h>


DeviceSettings::DeviceSettings(const char *devicename) :
  Menu("Device"),
  DeviceName(*this, "DeviceName", devicename) {
}
