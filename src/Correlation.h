/*
  Correlation - Analyzer for detecting negative correlations.
  Created by Jan Benda, June 19th, 2022.
*/

#ifndef Correlation_h
#define Correlation_h


#include <Arduino.h>
#include <AudioMonitor.h>
#include <Analyzer.h>


class Correlation : public Analyzer {

 public:

  // Construct correlation analyzer.
  Correlation(AnalysisChain *chain=0);

  // Construct correlation analyzer with audio feedback on channel feedback.
  Correlation(AudioMonitor *audio, uint8_t feedback, AnalysisChain *chain=0);

  // Initialize.
  virtual void start(uint8_t nchannels, size_t nframes);
  
  // Compute and check correlations.
  virtual void analyze(sample_t **data, uint8_t nchannels, size_t nframes);

  // Correlation between the first two data traces.
  float correlation() const;

  // Contrast between standard deviations of the first two data traces.
  float contrast() const;


 protected:

  float Corr;
  float Contrast;
  AudioMonitor *Audio;
  uint8_t AudioFeedback;
  
};


#endif
