#include <Clipping.h>


Clipping::Clipping(int channel, AnalysisChain *chain) :
  Analyzer(chain),
  Channel(channel),
  ClipThreshold(0.75),
  MuteThreshold(10.0),
  ClippedAboveFrac(0.0),
  ClippedBelowFrac(0.0),
  ClippedFrac(0.0),
  Audio(0),
  AudioFeedbackChannel(0),
  AudioFeedback(true) {
}


Clipping::Clipping(int channel, AudioMonitor *audio, uint8_t feedback,
		   AnalysisChain *chain) :
  Analyzer(chain),
  Channel(channel),
  ClipThreshold(0.75),
  MuteThreshold(10.0),
  ClippedAboveFrac(0.0),
  ClippedBelowFrac(0.0),
  ClippedFrac(0.0),
  Audio(audio),
  AudioFeedbackChannel(feedback),
  AudioFeedback(true) {
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
  for (size_t i=0; i<nframes; i++) {
    if (data[Channel][i] > ClipThreshold)
      nover++;
    else if (data[Channel][i] < -ClipThreshold)
      nunder++;
  }
  ClippedAboveFrac = float(nover)/nframes/nchannels;
  ClippedBelowFrac = float(nunder)/nframes/nchannels;
  ClippedFrac = ClippedAboveFrac + ClippedBelowFrac;
  if (Audio != 0) {
    if (AudioFeedback)
      Audio->setFeedback(ClippedFrac, AudioFeedbackChannel);
    if (ClippedFrac > MuteThreshold)
      Audio->pause();
    else
      Audio->play();
  }
}


void Clipping::setFeedback(bool feedback) {
  AudioFeedback = feedback;
  if (!AudioFeedback)
    Audio->setFeedback(0.0, AudioFeedbackChannel);
}


void Clipping::toggleFeedback() {
  setFeedback(!AudioFeedback);
}


bool Clipping::feedbackEnabled() const {
  return AudioFeedback;
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

