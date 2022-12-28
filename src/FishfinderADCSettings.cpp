#include <FishfinderADCSettings.h>


FishfinderADCSettings::FishfinderADCSettings(const char *name, uint32_t rate,
					     uint8_t bits, uint8_t averaging,
					     ADC_CONVERSION_SPEED conversion_speed,
					     ADC_SAMPLING_SPEED sampling_speed,
					     ADC_REFERENCE reference,
					     float analysis_interval,
					     float analysis_window) :
  TeensyADCSettings(name, rate, bits, averaging, conversion_speed,
		    sampling_speed, reference),
  AnalysisInterval(analysis_interval),
  AnalysisWindow(analysis_window) {
}


void FishfinderADCSettings::setAnalysisInterval(float analysis_interval) {
  AnalysisInterval = analysis_interval;
}


void FishfinderADCSettings::setAnalysisWindow(float analysis_window) {
  AnalysisWindow = analysis_window;
}


void FishfinderADCSettings::configure(const char *key, const char *val) {
  char pval[30];
  if (strcmp(key, "analysisinterval") == 0) {
    setAnalysisInterval(parseTime(val));
    sprintf(pval, "%.0fms", 1000.0*AnalysisInterval);
  }
  else if (strcmp(key, "analysiswindow") == 0) {
    setAnalysisWindow(parseTime(val));
    sprintf(pval, "%.0fms", 1000.0*AnalysisWindow);
  }
  else {
    TeensyADCSettings::configure(key, val);
    return;
  }
  Serial.printf("  set %s-%s to %s\n", name(), key, pval);
}


void FishfinderADCSettings::report() const {
  TeensyADCSettings::report();
  Serial.printf("  interval:   %.0fms\n", 1000.0*AnalysisInterval);
  Serial.printf("  window:     %.0fms\n", 1000.0*AnalysisWindow);
}
