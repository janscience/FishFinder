#include <LoggerSettings.h>


LoggerSettings::LoggerSettings(const char *path, const char *filename,
			       int mode, float filetime, float initialdelay) :
  FishfinderSettings(path, filename, mode),
  FileTime(*this, "FileTime", filetime, "%.0f", "s"),
  InitialDelay(*this, "InitialDelay", initialdelay, "%.0f", "s") {
  setName("Logger");
}

