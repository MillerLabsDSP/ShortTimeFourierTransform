/*
  ==============================================================================

    FFTProcessor.cpp
    Created: 17 Aug 2024 9:34:54pm
    Author:  Zachary Miller

  ==============================================================================
*/

#include "FFTProcessor.h"

FFTProcessor::FFTProcessor() :
    fft(fftOrder)
//    window(fftSize + 1, juce::dsp::WindowingFunction<float>::hann, false)
{
    switch (windowType) {
        case Hanning:
            hannWindow(window.data(), fftSize);
            break;
    }
}

void FFTProcessor::reset() {
    count = 0;
    pos = 0;
    std::fill(inputFifo.begin(), inputFifo.end(), 0.0f);
    std::fill(outputFifo.begin(), outputFifo.end(), 0.0f);
}

void FFTProcessor::processSpectrum(float* data, int numBins) {
    auto* cdata = reinterpret_cast<std::complex<float>*>(data);
    
    for (int i = 0; i < numBins; ++i) { // i = individual frequency bin
        float magnitude = std::abs(cdata[i]); // real
        float phase = std::arg(cdata[i]); // imaginary
        
        // Do spectral processing here!..
        
        cdata[i] = std::polar(magnitude, phase); // return complex val
    }
}

void FFTProcessor::processFrame(bool bypassed) {
    const float* inputPtr = inputFifo.data(); // pointer to inputFifo memory array
    float* fftPtr = fftData.data(); // pointer to fftPtr memory array
    
    std::memcpy(fftPtr, inputPtr + pos, (fftSize - pos) * sizeof(float));
        if (pos > 0) {
            std::memcpy(fftPtr + fftSize - pos, inputPtr, pos * sizeof(float));
        }
    
//    window.multiplyWithWindowingTable(fftPtr, fftSize); // 1st window: Before FFT
    for (int i = 0; i < fftSize; ++i) { // Custom windowing
            fftPtr[i] *= window[i];
        }
    
    if (!bypassed) {
        fft.performRealOnlyForwardTransform(fftPtr, true);
        processSpectrum(fftPtr, numBins);
        fft.performRealOnlyInverseTransform(fftPtr);
    }
    
//    window.multiplyWithWindowingTable(fftPtr, fftSize); // 2nd window (window^2): After FFT
    for (int i = 0; i < fftSize; ++i) { // Custom windowing
            fftPtr[i] *= window[i];
        }
    
    for (int i = 0; i < fftSize; ++i) {
        fftPtr[i] *= windowCorrection;
    }
    
    for (int i = 0; i < pos; ++i) {
            outputFifo[i] += fftData[i + fftSize - pos];
        }
        for (int i = 0; i < fftSize - pos; ++i) {
            outputFifo[i + pos] += fftData[i];
        }
}

float FFTProcessor::processSample(float& sample, bool bypassed) {
    inputFifo[pos] = sample;
    float outputSample = outputFifo[pos];
    outputFifo[pos] = 0.f;
    
    pos += 1;
    if (pos == fftSize) {
        pos = 0;
    }
    
    count += 1;
    if (count == hopSize) {
        count = 0;
        processFrame(bypassed);
    }
    
    return outputSample;
}
