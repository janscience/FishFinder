/*
  FishfinderADCSettings - configuration settings for ADC of fishfinder.
  Created by Jan Benda, Dec 28th, 2022.
*/


#ifndef FishfinderADCSettings_h
#define FishfinderADCSettings_h


#include <Arduino.h>
#include <InputADCSettings.h>


class FishfinderADCSettings : public InputADCSettings {

public:
  // Constructor setting configuration name.
  FishfinderADCSettings(const char *name, int8_t channel1=-1, int8_t channel2=-1,
			uint32_t rate=0, uint8_t bits=16, uint8_t averaging=4,
			ADC_CONVERSION_SPEED conversion_speed=ADC_CONVERSION_SPEED::HIGH_SPEED,
			ADC_SAMPLING_SPEED sampling_speed=ADC_SAMPLING_SPEED::HIGH_SPEED,
			ADC_REFERENCE reference=ADC_REFERENCE::REF_3V3,
			float analysis_interval=0.0, float analysis_window=0.0);

  // Pin number of channel 1.
  int channel1() const { return Channel1; };

  // Set pin of channel 1. If -1, do not use this channel.
  void setChannel1(int8_t channel1);

  // Pin number of channel 2.
  int channel2() const { return Channel2; };

  // Set pin of channel 2. If -1, do not use this channel.
  void setChannel2(int8_t channel2);

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

  // Apply ADC settings on adc.
  // If no adc is provided, the one provided to the constructor is used.
  void configure(InputADC *adc=0);

  // Report current settings on Serial.
  void report() const;

    
protected:

  int8_t Channel1;
  int8_t Channel2;
  float AnalysisInterval;
  float AnalysisWindow;
  
};

#endif

