#include <arm_math.h>
#include <Plotting.h>


Plotting::Plotting(Display *screen, AnalysisChain *chain) :
  Analyzer(chain),
  Screen(screen),
  Window(0.01),
  MaxCounter(1),
  Counter(0) {
}


void Plotting::setWindow(float time) {
  if (time < 0.001)
    time = 0.001;
  Window = time;
}


void Plotting::setSkipping(int skip) {
  if (skip < 1)
    skip = 1;
  MaxCounter = skip;
}


void Plotting::start() {
  Counter = 0;
}


void Plotting::analyze(float **data, uint8_t nchannels, size_t nframes) {
  if (Counter == 0) {
    int nn = int(Window*Rate);
    if (nn > nframes)
      nn = nframes;
    // compute difference:
    float data_diff[nframes];
    arm_add_f32(data[0], data[1], data_diff, nn);
    arm_scale_f32(data_diff, 0.5, data_diff, nn);
    // plot:
    Screen->clearPlots();
    Screen->plot(0, data_diff, nn, 0);
    //Screen->plot(0, data[0], nn, 0);
  }
  Counter++;
  if (Counter >= MaxCounter)
    Counter = 0;
}

