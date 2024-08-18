/*
  ==============================================================================

    FFTProcessor.h
    Created: 17 Aug 2024 9:34:54pm
    Author:  Zachary Miller

  ==============================================================================
*/

#pragma once
#include <juce_dsp/juce_dsp.h>

class FFTProcessor {
public:
    
    enum WindowType {
        Hanning
    };
    
    FFTProcessor();
    ~FFTProcessor();
    int getLatencyInSamples() const;
    void reset();
    float processSample(float& sample, bool bypassed);
    void processBlock(float* data, int numSamples, bool bypassed);
        
private:
    
    WindowType windowType = Hanning;
    
    // -------------------- WINDOW TYPES --------------------
    static void hannWindow(float* window, int length) {
        float delta = 6.28318500518798828125f / float(length);
        float phase = 0.f;
        for (int i = 0; i < length; ++i) {
            window[i] = 0.5f * (1.f - std::cos(phase));
            phase += delta;
        }
    }
    // ------------------------------------------------------
    
    void processSpectrum(float* data, int numBins);
    void processFrame(bool bypassed);
        
    // FFT and Hanning variables
    static constexpr int fftOrder = 10;
    static constexpr int fftSize = 1 << fftOrder; // 1 bitshifted by 10 = 1024 samples
    static constexpr int numBins = fftSize / 2 + 1; // 513 bins
    static constexpr int overlap = 4; // 75% overlap
    static constexpr int hopSize = fftSize / overlap; // 256 samples
    static constexpr float windowCorrection = 2.f / 3.f;
    
    // JUCE DSP FFT and Hann window
    juce::dsp::FFT fft;
//    juce::dsp::WindowingFunction<float> window;
    
    // Custom windowing alternative that isnt JUCE DSP
     std::array<float, fftSize> window;
    
    // Circular buffer variables
    int count = 0;
    int pos = 0;
    std::array<float, fftSize> inputFifo;
    std::array<float, fftSize> outputFifo;
    std::array<float, fftSize * 2> fftData; // Note that complex numbers take up two floats
};
