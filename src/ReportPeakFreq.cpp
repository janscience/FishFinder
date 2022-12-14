#include <ReportPeakFreq.h>


ReportPeakFreq::ReportPeakFreq(Spectrum *spec, Display *screen, int textarea,
			       AnalysisChain *chain) :
  Analyzer(chain),
  Screen(screen),
  TextArea(textarea),
  FMin(0.0),
  FMax(1.0e9),
  Spec(spec),
  PeakFreq(0.0) {
}


void ReportPeakFreq::setFrequencyRange(float fmin, float fmax) {
  if (fmin < fmax) {
    FMin = fmin;
    FMax = fmax;
  }
}


void ReportPeakFreq::start(uint8_t nchannels, size_t nframes) {
  PeakFreq = 0.0;
}


void ReportPeakFreq::analyze(sample_t **data, uint8_t nchannels, size_t nframes) {
  if (!Spec->changed())
    return;
  // maximum power:
  uint32_t i0 = 0;
  uint32_t i1 = Spec->nfft()/2;
  if (FMin > 0.0)
    i0 = uint32_t(FMin/Spec->resolution());
  if (FMax < 1.0e9)
    i1 = min(i1, uint32_t(FMax/Spec->resolution()));
  q15_t max;
  uint32_t index;
  arm_max_q15(Spec->power() + i0, i1-i0, &max, &index);
  char fs[10];
  if (index > 0) {
    PeakFreq = float(i0 + index)*Spec->resolution();
    sprintf(fs, "%.0fHz", PeakFreq);
  }
  else
    strcpy(fs, "");
  // report:
  Screen->writeText(TextArea, fs);
}

