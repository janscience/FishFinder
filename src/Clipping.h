/*
  Clipping - Analyzer for detecting and reporting clipping events.
  Created by Jan Benda, June 19th, 2022.
*/

#ifndef Clipping_h
#define Clipping_h


#include <Arduino.h>
#include <AudioMonitor.h>
#include <Analyzer.h>


class Clipping : public Analyzer {

 public:

  // Construct clipping analyzer.
  Clipping(AudioMonitor *audio=0, AnalysisChain *chain=0);

  // Set clipping threshold (0-1). Default is 0.75.
  void setThreshold(float thresh);

  // Check for clipping.
  virtual void analyze(float **data, uint8_t nchannels, size_t nframes);


 protected:

  float Threshold;
  AudioMonitor *Audio;
  
};


#endif
