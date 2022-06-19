#include <Clipping.h>


Clipping::Clipping(AudioMonitor *audio, AnalysisChain *chain) :
  Analyzer(chain),
  Threshold(0.75),
  Audio(audio) {
}


void Clipping::setThreshold(float thresh) {
  Threshold = thresh;
}


void Clipping::analyze(float **data, uint8_t nchannels, size_t nframes) {
  int nover = 0;
  for (uint8_t c=0; c<nchannels; c++) {
    for (size_t i=0; i<nframes; i++) {
      if (data[c][i] > Threshold || data[c][i] < -Threshold)
	nover++;
    }
  }
  float frac = float(nover)/nframes/nchannels;
  if (Audio != 0) {
    if (frac > 0.0001)
      Audio->setFeedbackInterval(500 - frac*300, 0);
    else
      Audio->setFeedbackInterval(0, 0);
  }
}

