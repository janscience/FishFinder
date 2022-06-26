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
  // set up of data collection:
  BufferIndex = 0;
  Buffer = (float *)malloc(NBuffer*sizeof(float));
  if (Buffer == 0) {
    Serial.println("Not enough memory to allocate spectrum buffer!");
    while (1) {};
  }
  float rate = Resolution * NBuffer;
  Step = 1;
  if (rate < Rate)
    Step = round(Rate/rate);
  Offs = 0;
  // initialize FFT:
  arm_status status = ARM_MATH_SUCCESS;
  status = arm_rfft_init_f32(&RFFT, &CFFT, NBuffer, 0, 1); // Teensy 3
  // status = arm_rfft_fast_init_f32(&RFFT, NBuffer); // Teensy 4
  if (status != ARM_MATH_SUCCESS) {
    Serial.println("Failed to initialize FFT!");
    while (1) {};
  }
}


void Spectrum::stop() {
  if (Buffer != 0)
    free(Buffer);
  Buffer = 0;
  BufferIndex = 0;
}


void Spectrum::analyze(float **data, uint8_t nchannels, size_t nframes) {
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
    float fft_buffer[NBuffer];
    arm_rfft_f32(&RFFT, Buffer, fft_buffer); // Teensy 3
    // arm_rfft_fast_f32(&RFFT, Buffer, fft_buffer, 0); // Teensy 4
    // power:
    arm_cmplx_mag_squared_f32(fft_buffer, Buffer, NBuffer/2);
    /* Teensy 4:
    arm_cmplx_mag_squared_f32(fft_buffer+2, Buffer+1, NBuffer/2 - 1);
    Buffer[0] = fft_buffer[0]*fft_buffer[0];
    Buffer[NBuffer/2] = fft_buffer[1]*fft_buffer[1];
    */
  }
}

