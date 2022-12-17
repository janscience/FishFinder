#include <LoggerSettings.h>


LoggerSettings::LoggerSettings(const char *path, const char *filename,
			       float filetime, float initialdelay) :
  Settings(path, filename) {
  setName("Logger");
  FileTime = filetime;
  InitialDelay = initialdelay;
}


void LoggerSettings::configure(const char *key, const char *val) {
  char pval[MaxStr];
  if (strcmp(key, "path") == 0) {
    strncpy(Path, val, MaxStr);
    strcpy(pval, Path);
  }
  else if (strcmp(key, "filename") == 0) {
    char *sp = strrchr(val, '.');
    if (sp != NULL)
      *sp = '\0';    // truncate file extension
    strncpy(FileName, val, MaxStr);
    strcpy(pval, FileName);
  }
  else if (strcmp(key, "filetime") == 0) {
    FileTime = parseTime(val);
    sprintf(pval, "%.0fs", FileTime);
  }
  else if (strcmp(key, "initialdelay") == 0) {
    InitialDelay = parseTime(val);
    sprintf(pval, "%.1fs", InitialDelay);
  }
  else {
    Serial.printf("  Settings key \"%s\" not found.\n", key);
    return;
  }
  Serial.printf("  set Settings-%s to %s\n", key, pval);
}
