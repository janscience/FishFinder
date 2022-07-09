#include <arm_math.h>
#include <Correlation.h>


Correlation::Correlation(AnalysisChain *chain) :
  Analyzer(chain),
  Corr(0.0),
  Contrast(0.0),
  Audio(0),
  AudioFeedback(0) {
}


Correlation::Correlation(AudioMonitor *audio, uint8_t feedback,
			 AnalysisChain *chain) :
  Analyzer(chain),
  Corr(0.0),
  Contrast(0.0),
  Audio(audio),
  AudioFeedback(feedback) {
}


void Correlation::start(uint8_t nchannels, size_t nframes) {
  Corr = 0.0;
  Contrast = 0.0;
}


void Correlation::analyze(sample_t **data, uint8_t nchannels, size_t nframes) {
  if (nchannels < 2)
    return;
  // mean:
  q15_t mean0 = 0;
  q15_t mean1 = 0;
  arm_mean_q15(data[0], nframes, &mean0);
  arm_mean_q15(data[1], nframes, &mean1);
  // subtract mean:
  arm_offset_q15(data[0], -mean0, data[0], nframes);
  arm_offset_q15(data[1], -mean1, data[1], nframes);
  // standard deviations:
  q15_t std0_q15;
  q15_t std1_q15;
  arm_rms_q15(data[0], nframes, &std0_q15);
  arm_rms_q15(data[1], nframes, &std1_q15);
  float std0 = float(std0_q15)/(1 << 15);
  float std1 = float(std1_q15)/(1 << 15);
  // covariance:
  q15_t d12[nframes];
  arm_mult_q15(data[0], data[1], d12, nframes);
  q15_t covar;
  arm_mean_q15(d12, nframes, &covar);
  Corr = float(covar)/(1 << 15)/std0*std1;
  Contrast = abs(std0-std1)/(std0+std1);
  if (Audio != 0) {
    // costs:
    // cost <0: bad, 1: perfect
    float costcorr = (Corr - 0.2)/0.8;
    float costratio = 1.0 - Contrast;
    float cost = (costcorr + costratio)/2;
    Audio->setFeedback(cost, AudioFeedback);
  }
}


float Correlation::correlation() const {
  return Corr;
}


float Correlation::contrast() const {
  return Contrast;
}

