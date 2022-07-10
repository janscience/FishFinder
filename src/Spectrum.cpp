#include <Spectrum.h>


Spectrum::Spectrum(int channel, AnalysisChain *chain) :
  Analyzer(chain),
  Channel(channel),
  Step(1),
  Offs(0),
  NBuffer(2*1024),
  BufferIndex(0),
  Buffer(0),
  Power(0),
  Resolution(10.0),
  Changed(false) {
}


size_t Spectrum::nfft() const {
  return NBuffer/2;
}


void Spectrum::setNFFT(size_t nfft) {
  NBuffer = 2*nfft;
}


float Spectrum::resolution() const {
  return 2.0/float(NBuffer)*Rate/Step;
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
  Changed = false;
}


void Spectrum::stop() {
  if (Buffer != 0)
    free(Buffer);
  Buffer = 0;
  BufferIndex = 0;
  if (Power != 0)
    free(Power);
  Power = 0;
  Changed = false;
}


void Spectrum::analyze(sample_t **data, uint8_t nchannels, size_t nframes) {
  if (BufferIndex < NBuffer) {
    // copy data:
    size_t i = 0;
    for (i=Offs; i<nframes && BufferIndex < NBuffer; i+=Step) {
      // TODO: Nyquist low-pass filter
      Buffer[BufferIndex++] = data[Channel][i]; // real
      Buffer[BufferIndex++] = 0;                // imaginary
    }
    Offs = i - nframes;
    //Serial.println(Offs);
  }
  else {
    BufferIndex = 0;
    Offs = 0;
    // subtract mean:
    q15_t mean;
    arm_mean_q15(Buffer, NBuffer, &mean);
    mean *= 2;   // half of the data were imaginary numbers of value zero.
    // subtract mean from real parts:
    for (size_t i=0; i<NBuffer; i+=2)
      Buffer[i] -= mean;
    // apply window function: TODO
    // compute spectrum:
    arm_cfft_radix2_q15(&CFFT, Buffer);
    // power:
    arm_cmplx_mag_squared_q15(Buffer, Power, NBuffer/2);
    Changed = true;
  }
}


q15_t *Spectrum::power() const {
  return Power;
}


bool Spectrum::changed() const {
  bool c = Changed;
  Changed = false;
  return c;
}

