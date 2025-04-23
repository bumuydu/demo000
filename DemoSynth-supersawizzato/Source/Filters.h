#pragma once
#include <JuceHeader.h>

class LadderFilter : public dsp::LadderFilter<float>
{
public:
    LadderFilter(){}
    ~LadderFilter(){}
    
    void prepare(const dsp::ProcessSpec& spec)
        {
            dsp::LadderFilter<float>::prepare(spec);
            
            // default values
            dsp::LadderFilter<float>::setMode(dsp::LadderFilter<float>::Mode::LPF12);
            dsp::LadderFilter<float>::setCutoffFrequencyHz(1000.0f);
            dsp::LadderFilter<float>::setResonance(0.0f);
        }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LadderFilter)
};

class ReleaseFilter
{
public:
    ReleaseFilter(double defaultRelease = 0.7)
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
        
//        for (int smp = startSample; smp < endSample; ++smp)
//        {
//            // explicit casting float buffer to double -- old school but compact code
//            envelope = jmax((double)bufferData[smp], envelope * alpha);
//            bufferData[smp] = (float)envelope;
//        }
        
        for (int smp = startSample; smp < endSample; ++smp)
        {
            if (envelope > 0.0001)
            {
                bufferData[smp] = (float)envelope;
                envelope *= alpha;
            }
            else
            {
                bufferData[smp] = 0.0f;
                envelope = 0.0;
            }
        }
        
    }
    
    void noteOn()
    {
        envelope = 1;
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
    StereoFilter(){}
    
    ~StereoFilter(){}
    
    void prepareToPlay(/*double sr, */const dsp::ProcessSpec& spec)
    {
        for (int i = 0; i < numFilters; ++i)
        {
            filters[i].prepare(spec);
            filters[i].setResonance(0.2f);
        }
        filters[0].setMode(dsp::LadderFilter<float>::Mode::LPF12);
        filters[0].setCutoffFrequencyHz(spec.sampleRate * 0.499);
        filters[1].setMode(dsp::LadderFilter<float>::Mode::HPF12);
        filters[1].setCutoffFrequencyHz(10.0f);
        setFrequency(0.5);
    }
    
    void processBlock(AudioBuffer<float>& buffer, const int numSamples)
    {
        dsp::AudioBlock<float> block{ buffer.getArrayOfWritePointers(), 1, (size_t)numSamples };
        dsp::ProcessContextReplacing<float> context{ block };
                
        filters[0].process(context);
        filters[1].process(context);
    }
    
    void setFrequency(const float newValue)
    {
        if (newValue < 0.5f)
        {     // low-pass
            float frequency = juce::jmap(newValue, 0.0f, 0.5f, 80.0f, 18000.0f);
            filters[0].setCutoffFrequencyHz(frequency);
        }else
        {     // high-pass
            float frequency = juce::jmap(newValue, 0.5f, 1.0f, 0.1f, 800.0f);
            filters[1].setCutoffFrequencyHz(frequency);
        }
    }
    
    void setQuality(const double newValue)
    {
        for (int i = 0; i < numFilters; ++i)
        {
            filters[i].setResonance(newValue);
        }
    }
    
private:
    
    // one LP one HP filter
    static const int numFilters = 2;
    dsp::LadderFilter<float> filters[numFilters];
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoFilter)
};
