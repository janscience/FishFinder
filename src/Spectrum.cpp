#include <Spectrum.h>


Spectrum::Spectrum(AnalysisChain *chain) :
  Analyzer(chain),
  Step(1),
  Offs(0),
  NBuffer(2048),
  BufferIndex(0),
  Buffer(0),
  Resolution(10.0) {
}


void Spectrum::setNFFT(size_t nfft) {
  NBuffer = nfft;
}


void Spectrum::setResolution(float freq) {
  Resolution = freq;
}


void Spectrum::start(uint8_t nchannels, size_t nframes) {
  // initialize FFT:
  arm_status status = ARM_MATH_SUCCESS;
  status = arm_cfft_radix2_init_q15(&CFFT, 1024, 0, 1);
  if (status != ARM_MATH_SUCCESS) {
    Serial.println("Failed to initialize FFT!");
    while (1) {};
  }
  // set up of data collection:
  BufferIndex = 0;
  Buffer = (sample_t *)malloc(NBuffer*sizeof(sample_t));
  if (Buffer == 0) {
    Serial.println("Not enough memory to allocate spectrum buffer!");
    while (1) {};
  }
  float rate = Resolution * NBuffer;
  Step = 1;
  if (rate < Rate)
    Step = round(Rate/rate);
  Offs = 0;
}


void Spectrum::stop() {
  if (Buffer != 0)
    free(Buffer);
  Buffer = 0;
  BufferIndex = 0;
}


void Spectrum::analyze(sample_t **data, uint8_t nchannels, size_t nframes) {
  int c = 0;   // channel to be analyzed
  if (BufferIndex < NBuffer) {
    // copy data:
    size_t i = 0;
    for (i=Offs; i<nframes && BufferIndex < NBuffer; i+=Step)
      Buffer[BufferIndex++] = data[c][i];
    Offs = i - nframes;
  }
  else {
    BufferIndex = 0;
    Offs = 0;
    // compute spectrum:
    arm_cfft_radix2_q15(&CFFT, Buffer);
    // power:
    arm_cmplx_mag_squared_q15(Buffer, Buffer, NBuffer/2);
  }
}

