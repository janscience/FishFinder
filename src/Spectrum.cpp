#include <Spectrum.h>


Spectrum::Spectrum(int channel, AnalysisChain *chain) :
  Analyzer(chain),
  Channel(channel),
  Step(1),
  Offs(0),
  NFFT(1024),
  BufferIndex(0),
  Buffer(0),
  Window(0),
  X(0),
  Power(0),
  Resolution(10.0),
  Changed(false) {
}


size_t Spectrum::nfft() const {
  return NFFT;
}


void Spectrum::setNFFT(size_t nfft) {
  NFFT = nfft;
}


float Spectrum::resolution() const {
  return 1.0/float(NFFT)*Rate/Step;
}


void Spectrum::setResolution(float freq) {
  Resolution = freq;
}


void Spectrum::start(uint8_t nchannels, size_t nframes) {
  // initialize FFT:
  arm_status status = ARM_MATH_SUCCESS;
  status = arm_cfft_radix2_init_q15(&CFFT, NFFT, 0, 1);
  if (status != ARM_MATH_SUCCESS) {
    Serial.println("Failed to initialize FFT!");
    while (1) {};
  }
  // set up of data collection:
  BufferIndex = 0;
  Buffer = (sample_t *)malloc(2*NFFT*sizeof(sample_t));
  Window = (sample_t *)malloc(NFFT*sizeof(sample_t));
  Power = (q15_t *)malloc(NFFT*sizeof(q15_t));
  if (Buffer == 0 || Power == 0) {
    Serial.println("Not enough memory to allocate spectrum buffer!");
    while (1) {};
  }
  // Hanning window:
  float a = TWO_PI/(NFFT-1);
  sample_t *window = Window;
  for (size_t j=0; j<NFFT; j++)
    *window++ = (1.0 - cos(a*j)) * (1<<14);
  float rate = Resolution * NFFT;
  Step = 1;
  if (rate < Rate)
    Step = round(Rate/rate);
  Offs = 0;
  Changed = false;
}


void Spectrum::stop() {
  if (Window != 0)
    free(Window);
  if (Buffer != 0)
    free(Buffer);
  Buffer = 0;
  Window = 0;
  BufferIndex = 0;
  if (Power != 0)
    free(Power);
  Power = 0;
  Changed = false;
}


void Spectrum::analyze(sample_t **data, uint8_t nchannels, size_t nframes) {
  if (BufferIndex < 2*NFFT) {
    // copy data: 2ms
    if (BufferIndex == 0)
      X = data[Channel][Offs];
    size_t i = 0;
    for (i=0; i<Offs && BufferIndex < 2*NFFT; i++)
      X += (data[Channel][i] - X)/Step;          // Nyquist low-pass filter
    for (i=Offs; i<nframes && BufferIndex < 2*NFFT; i+=Step) {
      Buffer[BufferIndex++] = X;  // real
      Buffer[BufferIndex++] = 0;  // imaginary
      for (size_t j=0; j<Step && i+j < nframes; j++)
	X += (data[Channel][i+j] - X)/Step;      // Nyquist low-pass filter
    }
    Offs = i - nframes;
  }
  else {
    // FFT: 5ms
    BufferIndex = 0;
    Offs = 0;
    // subtract mean:
    q15_t mean;
    arm_mean_q15(Buffer, 2*NFFT, &mean);
    mean *= 2;   // half of the data were imaginary numbers of value zero.
    // subtract mean from real parts and apply hanning window:
    sample_t *buffer = Buffer;
    sample_t *window = Window;
    for (size_t i=0; i<NFFT; i++) {
      *buffer -= mean;
      int32_t x = (int32_t)(*buffer) * (int32_t)(*window++);
      *buffer = x >> 15;
      buffer += 2;
    }
    // compute spectrum:
    arm_cfft_radix2_q15(&CFFT, Buffer);
    // power:
    arm_cmplx_mag_squared_q15(Buffer, Power, NFFT);
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

