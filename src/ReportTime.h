/*
  ReportTime - Analyzer for reporting the current time on screen.
  Created by Jan Benda, June 19th, 2022.
*/

#ifndef ReportTime_h
#define ReportTime_h


#include <Arduino.h>
#include <Display.h>
#include <RTClock.h>
#include <Analyzer.h>


class ReportTime : public Analyzer {

 public:

  // Construct time reporter.
  ReportTime(Display *screen, int textarea, RTClock *rtc,
	     AnalysisChain *chain=0);

  // Start analyzer.
  virtual void start(uint8_t nchannels, size_t nframes);

  // Report time on screen.
  virtual void analyze(float **data, uint8_t nchannels, size_t nframes);


 protected:

  Display *Screen;
  int TextArea;
  RTClock *RTC;
  char PrevTime[20];
  
};


#endif
