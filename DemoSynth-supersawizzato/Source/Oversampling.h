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
    
    void prepareToPlay(const int sampleRateOs, const int sampleRate, const int samplesPerBlockOs)
    {
//        antialiasingFilterL.reset();
//        antialiasingFilterR.reset();
//        halfBandCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRateOs, jmin(20000.0, sampleRate * 0.4999));
//        antialiasingFilterL.coefficients = halfBandCoeffs;
//        antialiasingFilterR.coefficients = halfBandCoeffs;
        spec.sampleRate = sampleRateOs;
        spec.maximumBlockSize = samplesPerBlockOs;
        spec.numChannels = 1;

        antialiasingFilterL.prepare(spec);
        antialiasingFilterR.prepare(spec);
        
        
        auto cutoff = jmin(20000.0, sampleRate * 0.4999);

        // FIR
        // filter order determines steepness (and its latency).
        // higher order means a steeper filter, higher latency
                                    // latency = (order - 1) / 2
        size_t filterOrder = 64;    // 32~ sample --> approx. 0.7 ms at 44.1kHz or 0.66 ms at 47 kHz
        // modify: change the order to reduce aliasing but introduce more latency
        
        firCoeffs = juce::dsp::FilterDesign<float>::designFIRLowpassWindowMethod(
            cutoff,
            sampleRateOs,
            filterOrder,
            juce::dsp::WindowingFunction<float>::blackman
        );
        antialiasingFilterL.coefficients = firCoeffs.get();
        antialiasingFilterR.coefficients = firCoeffs.get();
        
        // IIR
//        iirCoeffs = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(
//            cutoff,
//            sampleRateOs,
//            filterOrder
//        );

//        antialiasingFilterL.prepare({(double)sampleRateOs, (juce::uint32)samplesPerBlock, 2});
//        antialiasingFilterR.prepare({(double)sampleRateOs, (juce::uint32)samplesPerBlock, 2});
//
        // doesn't work
//        antialiasingFilterL.coefficients = iirCoeffs.get();
//        antialiasingFilterR.coefficients = iirCoeffs.get();
    }
    
    void filterAndDecimate(AudioBuffer<float>& oversmpBuf, AudioBuffer<float>& output, const int startSampleOs,
                           const int numSamplesOs, const int oversamplingFactor)
    {
        auto* left = oversmpBuf.getWritePointer(0);
        auto* right = oversmpBuf.getWritePointer(1);
        auto* inL = oversmpBuf.getReadPointer(0);
        auto* inR = oversmpBuf.getReadPointer(1);

        const int endSampleOs = startSampleOs + numSamplesOs;
        // if coefficients change dynamically --> reset -- (they do not)
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
//            outL[i] = inL[i * oversamplingFactor];
//            outR[i] = inR[i * oversamplingFactor];
            outL[i] = left[i * oversamplingFactor];
            outR[i] = right[i * oversamplingFactor];
        }
    }
    
    void resetFilter()
    {
        antialiasingFilterL.reset();
        antialiasingFilterR.reset();
    }
    
private:
    juce::dsp::ProcessSpec spec;
//    juce::dsp::IIR::Filter<float> antialiasingFilterL, antialiasingFilterR;
//    juce::dsp::IIR::Coefficients<float>::Ptr halfBandCoeffs;

    // fir
    juce::dsp::FIR::Filter<float> antialiasingFilterL, antialiasingFilterR;
//    juce::dsp::FIR::Coefficients<float>::Ptr firCoeffs;
    juce::ReferenceCountedObjectPtr<juce::dsp::FIR::Coefficients<float>> firCoeffs;
//    juce::ReferenceCountedObjectPtr<juce::dsp::FIR::Coefficients<float>> firCoeffs;

    // iir
//    juce::dsp::IIR::Coefficients<float>::Ptr iirCoeffs;
//    juce::ReferenceCountedArray<IIRCoefficients> iirCoeffs;
//    juce::ReferenceCountedObjectPtr<juce::dsp::IIR::Coefficients<float>> iirCoeffs;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Oversampling)
};
