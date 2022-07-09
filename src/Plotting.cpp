#include <arm_math.h>
#include <Plotting.h>


Plotting::Plotting(Display *screen, AnalysisChain *chain) :
  Analyzer(chain),
  Screen(screen),
  Window(0.01),
  MaxCounter(1),
  Counter(0),
  Align(-1.0) {
}


void Plotting::setWindow(float time) {
  if (time < 0.001)
    time = 0.001;
  Window = time;
}


void Plotting::zoomIn() {
  Window *= 0.5;
  if (Window < 0.001)
    Window = 0.001;
}


void Plotting::zoomOut() {
  Window *= 2.0;
}


void Plotting::setSkipping(int skip) {
  if (skip < 1)
    skip = 1;
  MaxCounter = skip;
}


void Plotting::setAlignMax(float frac) {
  if (frac <= 1.0)
    Align = frac;
}


void Plotting::start(uint8_t nchannels, size_t nframes) {
  Counter = 0;
}


void Plotting::analyze(sample_t **data, uint8_t nchannels, size_t nframes) {
  if (Counter == 0) {
    sample_t *pdata = data[0];
    /*
    if (nchannels > 1) {
      // compute difference:
      sample_t data_diff[nframes];
      //arm_sub_q15(data[0], data[1], data_diff, nframes);
      arm_add_q15(data[0], data[1], data_diff, nframes); XXX Overflow!!!
      arm_scale_q15(data_diff, 0.5, data_diff, nframes);
      pdata = data_diff;
      // XXX data diff looks weired towards the end of the array!!!
    }
    */
    // select window:
    size_t nw = int(Window*Rate);
    if (nw > nframes) {
      nw = nframes;
      Window = nw/Rate;
    }
    uint32_t offs = 0;
    if (Align >= 0.0) {
      if (nw > nframes/2)
	nw = nframes/2;
      int start = (1.0-Align)*nw;
      q15_t max;
      arm_max_q15(&(pdata[start]), 2*nw, &max, &offs);
    }
    // plot:
    Screen->clearPlots();
    Screen->plot(0, &(pdata[offs]), nw, 0);
    if (nchannels > 1)
      Screen->plot(0, &(data[1][offs]), nw, 1);
  }
  Counter++;
  if (Counter >= MaxCounter)
    Counter = 0;
}

