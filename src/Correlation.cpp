#include <arm_math.h>
#include <Correlation.h>


Correlation::Correlation(AudioMonitor *audio, AnalysisChain *chain) :
  Analyzer(chain),
  Audio(audio) {
}


void Correlation::analyze(float **data, uint8_t nchannels, size_t nframes) {
  if (nchannels < 1)
    return;
  // mean:
  float mean0 = 0.0;
  float mean1 = 0.0;
  arm_mean_f32(data[0], nframes, &mean0);
  arm_mean_f32(data[1], nframes, &mean1);
  // subtract mean:
  arm_offset_f32(data[0], -mean0, data[0], nframes);
  arm_offset_f32(data[1], -mean1, data[1], nframes);
  // standard deviations:
  float std0;
  float std1;
  arm_rms_f32(data[0], nframes, &std0);
  arm_rms_f32(data[1], nframes, &std1);
  // covariance:
  float d12[nframes];
  arm_mult_f32(data[0], data[1], d12, nframes);
  float covar;
  arm_mean_f32(d12, nframes, &covar);
  float corr = covar;
  corr /= std0*std1;
  // costs:
  // cost <0: bad, 1: perfect
  // float costcorr = (0.2 - corr)/0.8;
  float costcorr = (corr - 0.2)/0.8;
  float costratio = 1.0 - abs(std0-std1)/(std0+std1);
  //float costampl = 0.5*(std0 + std1)/0.7; // better some maxima
  float cost = (costcorr + costratio)/2;
  //float cost = (0.2 - corr)/0.8;  // <0: bad, 1: perfect
  //Serial.printf("%6.3f  %6.3f  %6.3f  %6.3f\n", corr, costcorr, costratio, cost);
  if (cost < 0.0)
    Audio->setFeedbackInterval(0, 1);
  else 
    Audio->setFeedbackInterval(500 - cost*300, 1);
}

