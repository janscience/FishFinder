#include <FishfinderSettings.h>


FishfinderSettings::FishfinderSettings(const char *path, const char *filename) :
  Configurable("Fishfinder") {
  strncpy(Path, path, MaxStr);
  strncpy(FileName, filename, MaxStr);
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
  else {
    Serial.printf("  Settings key \"%s\" not found.\n", key);
    return;
  }
  Serial.printf("  set Settings-%s to %s\n", key, pval);
}
