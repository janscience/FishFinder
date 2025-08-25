#include <FishfinderSettings.h>


const char *FishfinderSettings::ModeStrings[FishfinderSettings::MaxModes] = {
  "44khz", "48khz", "96khz", "192khz" };
  
const int FishfinderSettings::ModeEnums[FishfinderSettings::MaxModes] = {
  0, 1, 2, 3 };


FishfinderSettings::FishfinderSettings(const char *path, const char *filename,
				       int mode) :
  Menu("Fishfinder"),
  Path(*this, "Path", path),
  FileName(*this, "FileName", filename),
  Mode(*this, "Mode", mode, ModeEnums, ModeStrings, MaxModes) {
}
