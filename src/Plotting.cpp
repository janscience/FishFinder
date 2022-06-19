#include <arm_math.h>
#include <Plotting.h>


Plotting::Plotting(Display *screen, AnalysisChain *chain) :
  Analyzer(chain),
  Screen(screen) {
}


void Plotting::analyze(float **data, uint8_t nchannels, size_t nframes) {
  int nn = nframes/5;
  // compute difference:
  float data_diff[nframes];
  arm_add_f32(data[0], data[1], data_diff, nn);
  arm_scale_f32(data_diff, 0.5, data_diff, nn);
  // plot:
  Screen->clearPlots();
  Screen->plot(0, data_diff, nn, 0);
  //Screen->plot(0, data[0], nn, 0);
}

