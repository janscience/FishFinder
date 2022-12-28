#include <LoggerSettings.h>


LoggerSettings::LoggerSettings(const char *path, const char *filename,
			       int mode, float filetime, float initialdelay) :
  FishfinderSettings(path, filename, mode) {
  setName("Logger");
  FileTime = filetime;
  InitialDelay = initialdelay;
}


void LoggerSettings::configure(const char *key, const char *val) {
  char pval[MaxStr];
  if (strcmp(key, "filetime") == 0) {
    FileTime = parseTime(val);
    sprintf(pval, "%.0fs", FileTime);
  }
  else if (strcmp(key, "initialdelay") == 0) {
    InitialDelay = parseTime(val);
    sprintf(pval, "%.1fs", InitialDelay);
  }
  else {
    FishfinderSettings::configure(key, val);
    return;
  }
  Serial.printf("  set %s-%s to %s\n", name(), key, pval);
}
