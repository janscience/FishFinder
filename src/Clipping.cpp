#include <Clipping.h>


Clipping::Clipping(AudioMonitor *audio, AnalysisChain *chain) :
  Analyzer(chain),
  ClipThreshold(0.75),
  MuteThreshold(10.0),
  Audio(audio) {
}


void Clipping::setClipThreshold(float thresh) {
  ClipThreshold = thresh;
}


void Clipping::setMuteThreshold(float thresh) {
  MuteThreshold = thresh;
}


void Clipping::analyze(float **data, uint8_t nchannels, size_t nframes) {
  int nover = 0;
  for (uint8_t c=0; c<nchannels; c++) {
    for (size_t i=0; i<nframes; i++) {
      if (data[c][i] > ClipThreshold || data[c][i] < -ClipThreshold)
	nover++;
    }
  }
  float frac = float(nover)/nframes/nchannels;
  Audio->setFeedback(frac, 0);
  if (frac > MuteThreshold)
    Audio->pause();
  else
    Audio->play();
}

