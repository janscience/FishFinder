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
  Correlation(AudioMonitor *audio=0, AnalysisChain *chain=0);

  // Compute and check correlations.
  virtual void analyze(float **data, uint8_t nchannels, size_t nframes);


 protected:

  AudioMonitor *Audio;
  
};


#endif
