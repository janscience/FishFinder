#include <Clipping.h>


Clipping::Clipping(AnalysisChain *chain) :
  Analyzer(chain),
  ClipThreshold(0.75),
  MuteThreshold(10.0),
  ClippedAboveFrac(0.0),
  ClippedBelowFrac(0.0),
  ClippedFrac(0.0),
  Audio(0),
  AudioFeedback(0) {
}


Clipping::Clipping(AudioMonitor *audio, uint8_t feedback,
		   AnalysisChain *chain) :
  Analyzer(chain),
  ClipThreshold(0.75),
  MuteThreshold(10.0),
  ClippedAboveFrac(0.0),
  ClippedBelowFrac(0.0),
  ClippedFrac(0.0),
  Audio(audio),
  AudioFeedback(feedback) {
}


void Clipping::setClipThreshold(float thresh) {
  ClipThreshold = thresh * (1 << 15);
}


void Clipping::setMuteThreshold(float thresh) {
  MuteThreshold = thresh;
}


void Clipping::start(uint8_t nchannels, size_t nframes) {
  ClippedAboveFrac = 0.0;
  ClippedBelowFrac = 0.0;
  ClippedFrac = 0.0;
}


void Clipping::analyze(sample_t **data, uint8_t nchannels, size_t nframes) {
  int nover = 0;
  int nunder = 0;
  for (uint8_t c=0; c<nchannels; c++) {
    for (size_t i=0; i<nframes; i++) {
      if (data[c][i] > ClipThreshold)
	nover++;
      else if (data[c][i] < -ClipThreshold)
	nunder++;
    }
  }
  ClippedAboveFrac = float(nover)/nframes/nchannels;
  ClippedBelowFrac = float(nunder)/nframes/nchannels;
  ClippedFrac = ClippedAboveFrac + ClippedBelowFrac;
  if (Audio != 0) {
    Audio->setFeedback(ClippedFrac, AudioFeedback);
    if (ClippedFrac > MuteThreshold)
      Audio->pause();
    else
      Audio->play();
  }
}


float Clipping::clippedAbove() const {
  return ClippedAboveFrac;
}


float Clipping::clippedBelow() const {
  return ClippedBelowFrac;
}


float Clipping::clipped() const {
  return ClippedFrac;
}

