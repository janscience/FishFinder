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

  // Construct clipping analyzer for specified channel.
  Clipping(int channel, AnalysisChain *chain=0);

  // Construct clipping analyzer for specified channel with audio
  // feedback on channel feedback.
  Clipping(int channel, AudioMonitor *audio, uint8_t feedback,
	   AnalysisChain *chain=0);

  // Set amplitude threshold for detecting clipped signals (0-1).
  // Default is 0.75.
  void setClipThreshold(float thresh);

  // Set threshold for muting as fraction of clipped data points.
  // Default is 1, i.e. do not mute.
  void setMuteThreshold(float thresh);

  // Initialize.
  virtual void start(uint8_t nchannels, size_t nframes);

  // Check for clipping.
  virtual void analyze(sample_t **data, uint8_t nchannels, size_t nframes);

  // Fraction of data points above clipping threshold.
  float clippedAbove() const;

  // Fraction of data points below negative clipping threshold.
  float clippedBelow() const;

  // Fraction of clipped data points, sum of clippedAbove() and clippedBelow().
  float clipped() const;


 protected:

  int Channel;
  sample_t ClipThreshold;
  float MuteThreshold;
  float ClippedAboveFrac;
  float ClippedBelowFrac;
  float ClippedFrac;
  AudioMonitor *Audio;
  uint8_t AudioFeedback;
  
};


#endif
