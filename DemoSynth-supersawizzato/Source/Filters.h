#pragma once
#include <JuceHeader.h>

#define MAX_NUM_CH 2

class LadderFilter : public dsp::LadderFilter<float>
{
public:
    LadderFilter(){}
    ~LadderFilter(){}
    
    void prepare(const dsp::ProcessSpec& spec)
    {
        dsp::LadderFilter<float>::prepare(spec);
        
        // default values
        //setEnabled(1);
        dsp::LadderFilter<float>::setMode(dsp::LadderFilter<float>::Mode::LPF12);
        dsp::LadderFilter<float>::setCutoffFrequencyHz(1000.0f);
        dsp::LadderFilter<float>::setResonance(0.0f);
    }
    
//    void setCutoff(const float newValue)
//    {
//        setCutoffFrequencyHz(newValue);
//    }
//    
//    void setQuality(const float newValue)
//    {
//        setResonance(newValue);
//    }
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LadderFilter)
};

class ReleaseFilter
{
public:
    ReleaseFilter(double defaultRelease = 0.25)
        : release(defaultRelease)
    {}
    
    ~ReleaseFilter(){}
    
    void prepareToPlay(double sr)
    {
        sampleRate = sr;
        updateAlpha();
    }
    
    void processBlock(AudioBuffer<float>& buffer, const int startSample, const int numSamples)
    {
        auto bufferData = buffer.getWritePointer(0);
        const int endSample = numSamples + startSample;
        
        for (int smp = startSample; smp < endSample; ++smp)
        {
            // explicit casting float buffer to double -- old school but compact code
            envelope = jmax((double)bufferData[smp], envelope * alpha);
            bufferData[smp] = (float)envelope;
        }
    }
    
    // check if the envelope has (nearly) finished
    bool isActive() const
    {
        return envelope > 0.001;
    }
    
    void setRelease(double newValue)
    {
        release = newValue;
        updateAlpha();
    }
    
private:
    void updateAlpha()
    {
        const auto n = jmax(1.0, release * sampleRate);
        // updated the alpha calculation 
        alpha = pow(0.001, 1.0 / n);
    }
    
    double release;
    double sampleRate = 1.0;
    double alpha = 0.0;
    double envelope = 0.0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReleaseFilter)
};

class StereoFilter
{
public:
    StereoFilter(double defaultFrequency = 1000.0, double defaultQuality = 1 / MathConstants<double>::sqrt2)
        : frequency(defaultFrequency), quality(defaultQuality)
    {
        for (int f = 0; f < MAX_NUM_CH; ++f)
            iirFilters.add(new dsp::IIR::Filter<float>());
    }
    
    ~StereoFilter(){}
    
    void prepareToPlay(double sr)
    {
        sampleRate = sr;
        reset();
        updateCoefficients();
    }
    
    void processBlock(AudioBuffer<float>& buffer, const int numSamples)
    {
        // AudioBlock - another audio wrapper which is even more lightweight than the context
        // it's like an audio buffer without utilities like gain level, gain change etc.
        dsp::AudioBlock<float> block(buffer.getArrayOfWritePointers(), buffer.getNumChannels(), numSamples);
        
        // context is a bit like an audiobuffer but much slimmer
        // has one or more data array(s) and info on if it is replacing or non-replacing
        // replacing = all audio data that we modify/generate replaces the old data
        // non-replacing = input buffer and output buffers are different
        //dsp::ProcessContextReplacing<float> context(block);
        
        
        //iirFilter.process(context);
        // we have more than one channel so
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            dsp::AudioBlock<float> chBlock = block.getSingleChannelBlock(ch);
            dsp::ProcessContextReplacing<float> context(chBlock);
            iirFilters.getUnchecked(ch)->process(context);
        }
            
    }
    
    void setFrequency(const double newValue)
    {
        // to be safe 0.499 rather than the absolute limit 0.5
        frequency = jmin(newValue, sampleRate * 0.499);
        updateCoefficients();
    }
    
    void setQuality(const double newValue)
    {
        quality = newValue;
        updateCoefficients();
    }
    
    void reset()
    {
        for (int f = iirFilters.size(); --f >= 0;)
            iirFilters.getUnchecked(f)->reset();
    }
    
private:
    
    void updateCoefficients()
    {
        // ESERCIZIO: SWITCH-CASE con tipo di filtro (e aggiunta del relativo setter e parametro)
        
        // use these makeLowPass, makeHighPass etc. to set the correct coefficients for our filter
        auto iirCoeffs = dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, frequency, quality);
        
        //iirFilter.coefficients = iirCoeffs;
        for (int f = iirFilters.size(); --f >= 0;)
            iirFilters.getUnchecked(f)->coefficients = iirCoeffs;
    }
    
    double frequency;
    double quality;
    
    double sampleRate = 1.0;
    
    // we use float because the audiobuffer is float and we do not want to
    // cast the buffer to double back and forth every time
    //dsp::IIR::Filter<float> iirFilter;
    // we need an array of these filters that will work individually on every channel
    OwnedArray<dsp::IIR::Filter<float>> iirFilters;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoFilter)
};
