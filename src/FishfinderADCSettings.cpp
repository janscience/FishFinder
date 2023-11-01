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
  Channel1(this, "Channel1", channel1, "%hd"),
  Channel2(this, "Channel2", channel2, "%hd"),
  AnalysisInterval(this, "AnalysisInterval",  analysis_interval, "%.0f", "s", "ms"),
  AnalysisWindow(this, "AnalysisWindow", analysis_window, "%.0f", "s", "ms") {
}


void FishfinderADCSettings::setChannel1(int8_t channel1) {
  Channel1.setValue(channel1);
}


void FishfinderADCSettings::setChannel2(int8_t channel2) {
  Channel2.setValue(channel2);
}


void FishfinderADCSettings::setAnalysisInterval(float analysis_interval) {
  AnalysisInterval.setValue(analysis_interval);
}


void FishfinderADCSettings::setAnalysisWindow(float analysis_window) {
  AnalysisWindow.setValue(analysis_window);
}


void FishfinderADCSettings::configure(InputADC *adc) {
  return InputADCSettings::configure(adc);
}


void FishfinderADCSettings::setConfiguration(InputADC *adc) {
  InputADCSettings::configure(adc);
  if (adc == 0)
    return;
  adc->setChannel(0, channel1());
}
