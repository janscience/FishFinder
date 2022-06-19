/*
  Plotting - Analyzer for plotting data on screen.
  Created by Jan Benda, June 19th, 2022.
*/

#ifndef Plotting_h
#define Plotting_h


#include <Arduino.h>
#include <Display.h>
#include <Analyzer.h>


class Plotting : public Analyzer {

 public:

  // Construct data plotting.
  Plotting(Display *screen, AnalysisChain *chain=0);

  // Plot data.
  virtual void analyze(float **data, uint8_t nchannels, size_t nframes);


 protected:

  Display *Screen;
  
};


#endif
