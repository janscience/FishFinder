#include <DeviceSettings.h>


DeviceSettings::DeviceSettings(const char *device_name) :
  Configurable("Device") {
  strncpy(DeviceName, device_name, MaxStr);
}


void DeviceSettings::configure(const char *key, const char *val) {
  char pval[MaxStr];
  if (strcmp(key, "name") == 0) {
    strncpy(DeviceName, val, MaxStr);
    strcpy(pval, DeviceName);
  }
  else {
    Serial.printf("  %s key \"%s\" not found.\n", name(), key);
    return;
  }
  Serial.printf("  set %s-%s to %s\n", name(), key, pval);
}
