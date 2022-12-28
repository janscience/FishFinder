/*
  FishfinderADCSettings - configuration settings for ADC of fishfinder.
  Created by Jan Benda, Dec 28th, 2022.
*/


#ifndef FishfinderADCSettings_h
#define FishfinderADCSettings_h


#include <Arduino.h>
#include <TeensyADCSettings.h>


class FishfinderADCSettings : public TeensyADCSettings {

public:
  // Constructor setting configuration name.
  FishfinderADCSettings(const char *name, uint32_t rate=0,
			uint8_t bits=16, uint8_t averaging=4,
			ADC_CONVERSION_SPEED conversion_speed=ADC_CONVERSION_SPEED::HIGH_SPEED,
			ADC_SAMPLING_SPEED sampling_speed=ADC_SAMPLING_SPEED::HIGH_SPEED,
			ADC_REFERENCE reference=ADC_REFERENCE::REF_3V3,
			float analysis_interval=0.0, float analysis_window=0.0);

  // Update interval of analysis chain in seconds.
  float analysisInterval() const { return AnalysisInterval; };

  // Set update interval of analysis chain in seconds.
  void setAnalysisInterval(float analysis_interval);

  // Data window used for analysis chain in seconds.
  float analysisWindow() const { return AnalysisWindow; };

  // Set data window used for analysis chain in seconds.
  void setAnalysisWindow(float analysis_window);
  
  // Configure fishfinder ADC settings with the provided key-value pair.
  virtual void configure(const char *key, const char *val);

  // Report current settings on Serial.
  void report() const;

    
protected:

  float AnalysisInterval;
  float AnalysisWindow;
  
};

#endif

