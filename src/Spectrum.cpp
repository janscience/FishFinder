#include <Spectrum.h>


Spectrum::Spectrum(AnalysisChain *chain) :
  Analyzer(chain),
  Step(1),
  Offs(0),
  NBuffer(2*1024),
  BufferIndex(0),
  Buffer(0),
  Power(0),
  Resolution(10.0) {
}


void Spectrum::setNFFT(size_t nfft) {
  NBuffer = 2*nfft;
}


void Spectrum::setResolution(float freq) {
  Resolution = freq;
}


void Spectrum::start(uint8_t nchannels, size_t nframes) {
  // initialize FFT:
  arm_status status = ARM_MATH_SUCCESS;
  status = arm_cfft_radix2_init_q15(&CFFT, NBuffer/2, 0, 1);
  if (status != ARM_MATH_SUCCESS) {
    Serial.println("Failed to initialize FFT!");
    while (1) {};
  }
  // set up of data collection:
  BufferIndex = 0;
  Buffer = (sample_t *)malloc(NBuffer*sizeof(sample_t));
  Power = (q15_t *)malloc(NBuffer/2*sizeof(q15_t));
  if (Buffer == 0 || Power == 0) {
    Serial.println("Not enough memory to allocate spectrum buffer!");
    while (1) {};
  }
  float rate = Resolution * NBuffer/2;
  Step = 1;
  if (rate < Rate)
    Step = round(Rate/rate);
  Offs = 0;
  Serial.printf("NBuffer=%d\n", NBuffer);
  Serial.printf("Step=%d\n", Step);
  Serial.printf("Rate=%gHz\n", Rate/Step);
  Serial.printf("Resolution=%gHz\n", 2.0/float(NBuffer)*Rate/Step);
  Serial.printf("Duration=%gs\n", 0.5*float(NBuffer)/Rate*Step);
  Serial.println();
}


void Spectrum::stop() {
  if (Buffer != 0)
    free(Buffer);
  Buffer = 0;
  BufferIndex = 0;
  if (Power != 0)
    free(Power);
  Power = 0;
}


void Spectrum::analyze(sample_t **data, uint8_t nchannels, size_t nframes) {
  int c = 0;   // channel to be analyzed
  if (BufferIndex < NBuffer) {
    // copy data:
    size_t i = 0;
    for (i=Offs; i<nframes && BufferIndex < NBuffer; i+=Step) {
      Buffer[BufferIndex++] = data[c][i]; // real
      Buffer[BufferIndex++] = 0;          // imaginary
    }
    Offs = i - nframes;
    //Serial.println(Offs);
  }
  else {
    BufferIndex = 0;
    Offs = 0;
    // compute spectrum:
    arm_cfft_radix2_q15(&CFFT, Buffer);
    // power:
    arm_cmplx_mag_squared_q15(Buffer, Power, NBuffer/2);
    // maximum power:
    q15_t max;
    uint32_t index;
    arm_max_q15(Power, NBuffer/4, &max, &index);
    float max_freq = float(index)*2/NBuffer*Rate/Step;
    Serial.printf("freq=%.0fHz\n", max_freq);
  }
}

