#include <ReportTime.h>


ReportTime::ReportTime(Display *screen, int textarea, RTClock *rtc,
		       AnalysisChain *chain) :
  Analyzer(chain),
  Screen(screen),
  TextArea(textarea),
  RTC(rtc) {
  PrevTime[0] = '\0';
}


void ReportTime::start(uint8_t nchannels, size_t nframes) {
  PrevTime[0] = '\0';
}


void ReportTime::analyze(float **data, uint8_t nchannels, size_t nframes) {
  char ts[20];
  RTC->dateTime(ts);
  ts[strlen(ts)-3] = '\0';
  if (strcmp(PrevTime, ts) == 0)
    Screen->writeText(TextArea, ts);
  strcpy(PrevTime, ts);
}

