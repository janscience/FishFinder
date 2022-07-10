/*
  ReportPeakFreq - Analyzer for computing and reporting the peak frequency of a spectrum.
  Created by Jan Benda, July 10th, 2022.
*/

#ifndef ReportPeakFreq_h
#define ReportPeakFreq_h


#include <Arduino.h>
#include <Display.h>
#include <Spectrum.h>
#include <Analyzer.h>


class ReportPeakFreq : public Analyzer {

 public:

  // Construct peak frequency reporter.
  ReportPeakFreq(Spectrum *spec, Display *screen, int textarea,
		 AnalysisChain *chain=0);

  // Start analyzer.
  virtual void start(uint8_t nchannels, size_t nframes);

  // Report time on screen. This function does not use data.
  virtual void analyze(sample_t **data, uint8_t nchannels, size_t nframes);

  // Peak frequency of the current spectrum.
  void peakFrequency() const;


 protected:

  Display *Screen;
  int TextArea;
  Spectrum *Spec;
  float PeakFreq;
  
};


#endif
