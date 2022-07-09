#include <ReportPeakFreq.h>


ReportPeakFreq::ReportPeakFreq(Display *screen, int textarea, Spectrum *spec,
			       AnalysisChain *chain) :
  Analyzer(chain),
  Screen(screen),
  TextArea(textarea),
  Spec(spec),
  PeakFreq(0.0) {
}


void ReportPeakFreq::start(uint8_t nchannels, size_t nframes) {
  PeakFreq = 0.0;
}


void ReportPeakFreq::analyze(sample_t **data, uint8_t nchannels, size_t nframes) {
  if (!Spec->changed())
    return;
  // maximum power:
  q15_t max;
  uint32_t index;
  arm_max_q15(Spec->power(), Spec->nfft()/2, &max, &index);
  PeakFreq = float(index)*Spec->resolution();
  char fs[10];
  sprintf(fs, "%.0fHz", PeakFreq);
  Screen->writeText(TextArea, fs);
}

