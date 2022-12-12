#include <arm_math.h>
#include <Plotting.h>


const float Plotting::Windows[Plotting::MaxWindows] = {0.0001,
						       0.0002,
						       0.0005,
						       0.001,
						       0.002,
						       0.005,
						       0.01,
						       0.02,
						       0.05};  // in seconds


Plotting::Plotting(int channel, int color, Display *screen, int plotarea,
		   int timearea, int amplitudearea, AnalysisChain *chain) :
  Analyzer(chain),
  Channel(channel),
  Color(color),
  PlotArea(plotarea),
  TimeArea(timearea),
  AmplitudeArea(amplitudearea),
  Screen(screen),
  Window(0.01),
  WindowIndex(6),
  AmplitudeFac(1.0),
  MaxCounter(1),
  Counter(0),
  Align(-1.0) {
}


float Plotting::window() const {
  return Window;
}


void Plotting::setWindow(float time) {
  if (time < 0.001)
    time = 0.001;
  Window = time;
}


void Plotting::zoomTimeIn() {
  if (WindowIndex > 0)
    WindowIndex--;
  Window = Windows[WindowIndex];
  Counter = 0;
}


void Plotting::zoomTimeOut() {
  WindowIndex++;
  if (WindowIndex >= MaxWindows)
    WindowIndex = MaxWindows-1;
  Window = Windows[WindowIndex];
  Counter = 0;
}


void Plotting::zoomAmplitudeIn() {
  if (AmplitudeFac < 1 << 12)
    AmplitudeFac *= 2;
  Counter = 0;
}


void Plotting::zoomAmplitudeOut() {
  if (AmplitudeFac > 1.0)
    AmplitudeFac *= 0.5;
  Counter = 0;
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
    sample_t *pdata = data[Channel];
    /*
    if (nchannels > 1) {
      // compute difference: THIS SHOULD BE A SEPARATE ANALYZER!
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
      if (nw > nframes/2) {
	nw = nframes/2;
        Window = nw/Rate;
      }
      int start = (1.0-Align)*nw;
      q15_t max;
      arm_max_q15(&(pdata[start]), nframes-nw, &max, &offs);
    }
    // plot:
    Screen->clearPlots();
    Screen->setPlotZoom(PlotArea, AmplitudeFac);
    Screen->plot(PlotArea, &(pdata[offs]), nw, Color);
    // indicate time window:
    if (TimeArea >= 0) {
      char ts[10];
      if (Window >= 0.001)
	sprintf(ts, "%.0fms", 1000.0*Window);
      else
	sprintf(ts, "%.1fms", 1000.0*Window);
      Screen->writeText(TimeArea, ts);
    }
    // indicate zoom factor:
    if (AmplitudeArea >= 0) {
      char zs[10];
      sprintf(zs, "x%.0f", Screen->plotZoom(PlotArea));
      Screen->writeText(AmplitudeArea, zs);
    }
  }
  Counter++;
  if (Counter >= MaxCounter)
    Counter = 0;
}

