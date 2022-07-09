/*
  Spectrum - Analyzer for computing a power spectrum.
  Created by Jan Benda, June 26th, 2022.
*/

#ifndef Spectrum_h
#define Spectrum_h


#include <Arduino.h>
#include <Analyzer.h>
#include <arm_math.h>


class Spectrum : public Analyzer {

 public:

  // Construct spectrum analyzer.
  Spectrum(AnalysisChain *chain=0);

  // Number of samples used for FFT.
  size_t nfft() const;

  // Set number of FFT samples to nfft.
  // Must be one of 16, 32, 64, 128, 256, 512, 1024, 2048, 4096.
  void setNFFT(size_t nfft);

  // The actual resultion of the power spectrum in Hertz.
  float resolution() const;

  // Set desired frequency resolution to freq.
  void setResolution(float freq);

  // Initialize computation of power spectra.
  virtual void start(uint8_t nchannels, size_t nframes);

  // Free buffer.
  virtual void stop();

  // Compute power spectrum.
  virtual void analyze(sample_t **data, uint8_t nchannels, size_t nframes);

  // Pointer to a buffer of the power spectrum.
  // The buffer is nfft() samples long and has a resolution of
  // resolution() Hertz.
  q15_t *power() const;

  // Return true if a new power spectrum is available.
  bool changed() const;

  
protected:

  size_t Step;
  size_t Offs;
  size_t NBuffer;
  size_t BufferIndex;
  sample_t *Buffer;
  q15_t *Power;
  float Resolution;
  mutable bool Changed;
  arm_cfft_radix2_instance_q15 CFFT;
};


#endif
