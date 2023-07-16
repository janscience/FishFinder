#include <FishfinderSettings.h>


FishfinderSettings::FishfinderSettings(const char *path, const char *filename,
				       int mode) :
  Configurable("Fishfinder") {
  strncpy(Path, path, MaxStr);
  strncpy(FileName, filename, MaxStr);
  Mode = mode;
}


int FishfinderSettings::modeEnum(const char *mode) {
  if (strcmp(mode, "44khz") == 0)
    return 0;
  else if (strcmp(mode, "48khz") == 0)
    return 1;
  else if (strcmp(mode, "96khz") == 0)
    return 2;
  else if (strcmp(mode, "192khz") == 0)
    return 3;
  else
    return -1;
}


void FishfinderSettings::configure(const char *key, const char *val) {
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
  else if (strcmp(key, "mode") == 0) {
    Mode = modeEnum(val);
    strcpy(pval, val);
  }
  else {
    Serial.printf("  %s key \"%s\" not found.\n", name(), key);
    return;
  }
  Serial.printf("  set %s-%s to %s\n", name(), key, pval);
}
