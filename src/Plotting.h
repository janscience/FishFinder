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

  // Set width of plotting window to time seconds.
  // Default is 10ms.
  void setWindow(float time);

  // Plot only on every skip-th call of analyze().
  // Default is 1, i.e. plot on every call.
  void setSkipping(int skip);

  // Start plotting data.
  virtual void start();
  
  // Plot data.
  virtual void analyze(float **data, uint8_t nchannels, size_t nframes);


 protected:

  Display *Screen;
  float Window;
  int MaxCounter;
  int Counter;
  
};


#endif
