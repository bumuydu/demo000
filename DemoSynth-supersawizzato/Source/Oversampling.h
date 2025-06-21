/*
  ==============================================================================

    Oversampling.h
    Created: 23 May 2025 4:40:54pm
    Author:  Derin Donmez

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class Oversampling {
public:
    Oversampling(){}
    ~Oversampling(){};
    
    void prepareToPlay(const int sampleRateOs, const int sampleRate)
    {
        antialiasingFilterL.reset();
        antialiasingFilterR.reset();
        halfBandCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRateOs, jmin(20000.0, sampleRate * 0.4999));
        antialiasingFilterL.coefficients = halfBandCoeffs;
        antialiasingFilterR.coefficients = halfBandCoeffs;
    }
    
    void filterAndDecimate(AudioBuffer<float>& oversmpBuf, AudioBuffer<float>& output, const int startSampleOs,
                           const int numSamplesOs, const int oversamplingFactor)
    {
        auto* left = oversmpBuf.getWritePointer(0);
        auto* right = oversmpBuf.getWritePointer(1);
        auto* inL = oversmpBuf.getReadPointer(0);
        auto* inR = oversmpBuf.getReadPointer(1);

        const int endSampleOs = startSampleOs + numSamplesOs;
        // if coefficients change dynamically --> reset
//        iirFilters[ch].reset();
        
        // filter all samples of the oversampled buffer
        for (int smp = startSampleOs; smp < endSampleOs; ++smp)
        {
            left[smp] = antialiasingFilterL.processSample(inL[smp]);
            right[smp] = antialiasingFilterR.processSample(inR[smp]);
        }
        
        auto* outL = output.getWritePointer(0);
        auto* outR = output.getWritePointer(1);
        
        // decimate
        const int startSampleOriginal = startSampleOs / oversamplingFactor;
        const int numSamplesOriginal = numSamplesOs / oversamplingFactor;
        const int endSampleOriginal = startSampleOriginal + numSamplesOriginal;
        for (int i = startSampleOriginal; i < endSampleOriginal; ++i)
        {
            outL[i] = inL[i * oversamplingFactor];
            outR[i] = inR[i * oversamplingFactor];
        }
    }
    
    void resetFilter()
    {
        antialiasingFilterL.reset();
        antialiasingFilterR.reset();
    }
    
private:
    juce::dsp::IIR::Filter<float> antialiasingFilterL, antialiasingFilterR;
    juce::dsp::IIR::Coefficients<float>::Ptr halfBandCoeffs;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Oversampling)
};
