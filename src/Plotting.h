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

  // Construct data plotting of specified channel in color.
  Plotting(int channel, int color, Display *screen, int plotarea,
	   int textarea, AnalysisChain *chain=0);

  // The width of the plotting window in seconds.
  float window() const;

  // Set width of plotting window to time seconds.
  // Default is 10ms.
  void setWindow(float time);

  // Make width of plot window smaller by a factor of two.
  void zoomTimeIn();

  // Make width plot window larger by a factor of two.
  void zoomTimeOut();

  // Make plotted amplitude larger by a factor of two.
  void zoomAmplitudeIn();

  // Make plotted amplitude smaller by a factor of two.
  void zoomAmplitudeOut();

  // Plot only on every skip-th call of analyze().
  // Default is 1, i.e. plot on every call.
  void setSkipping(int skip);

  // Align the maximum of the data at frac (0-1) within the plot window.
  // Negative values (default) indicate not to align the data.
  void setAlignMax(float frac);

  // Start plotting data.
  virtual void start(uint8_t nchannels, size_t nframes);
  
  // Plot data.
  virtual void analyze(sample_t **data, uint8_t nchannels, size_t nframes);


 protected:

  int Channel;
  int Color;
  int PlotArea;
  int TextArea;
  Display *Screen;
  float Window;
  int WindowIndex;
  static const int MaxWindows = 9;
  static const float Windows[MaxWindows];
  float AmplitudeFac;
  int MaxCounter;
  int Counter;
  float Align;
  
};


#endif
