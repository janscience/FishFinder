#include <InputADC.h>
#include <FishfinderADCSettings.h>


FishfinderADCSettings::FishfinderADCSettings(const char *name, int8_t channel1, int8_t channel2,
					     uint32_t rate, uint8_t bits, uint8_t averaging,
					     ADC_CONVERSION_SPEED conversion_speed,
					     ADC_SAMPLING_SPEED sampling_speed,
					     ADC_REFERENCE reference,
					     float analysis_interval,
					     float analysis_window) :
  InputADCSettings(name, rate, bits, averaging, conversion_speed,
		   sampling_speed, reference),
  Channel1(channel1),
  Channel2(channel2),
  AnalysisInterval(analysis_interval),
  AnalysisWindow(analysis_window) {
}


void FishfinderADCSettings::setChannel1(int8_t channel1) {
  Channel1 = channel1;
}


void FishfinderADCSettings::setChannel2(int8_t channel2) {
  Channel2 = channel2;
}


void FishfinderADCSettings::setAnalysisInterval(float analysis_interval) {
  AnalysisInterval = analysis_interval;
}


void FishfinderADCSettings::setAnalysisWindow(float analysis_window) {
  AnalysisWindow = analysis_window;
}


void FishfinderADCSettings::configure(const char *key, const char *val) {
  char pval[30];
  if (strcmp(key, "channel1") == 0) {
  }
  else if (strcmp(key, "analysisinterval") == 0) {
    setAnalysisInterval(parseTime(val));
    sprintf(pval, "%.0fms", 1000.0*AnalysisInterval);
  }
  else if (strcmp(key, "analysiswindow") == 0) {
    setAnalysisWindow(parseTime(val));
    sprintf(pval, "%.0fms", 1000.0*AnalysisWindow);
  }
  else {
    InputADCSettings::configure(key, val);
    return;
  }
  Serial.printf("  set %s-%s to %s\n", name(), key, pval);
}


void FishfinderADCSettings::configure(InputADC *adc) {
  InputADCSettings::configure(adc);
  if (adc == 0)
    adc = ADC;
  if (adc == 0)
    return;
  adc->setChannel(0, channel1());
}


void FishfinderADCSettings::report() const {
  InputADCSettings::report();
  Serial.printf("  channel1:   %d\n", Channel1);
  Serial.printf("  channel2:   %d\n", Channel2);
  Serial.printf("  interval:   %.0fms\n", 1000.0*AnalysisInterval);
  Serial.printf("  window:     %.0fms\n", 1000.0*AnalysisWindow);
}
