#pragma once
#include <JuceHeader.h>

#define MAX_NUM_CH 2    // Da confermare! modify!

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
    
    void processBlock(AudioBuffer<float>& buffer, const int numSamples)
    {
        auto bufferData = buffer.getWritePointer(0);
        
        for (int smp = 0; smp < numSamples; ++smp)
        {
            // explicit casting float buffer to double -- old school but compact code
            envelope = jmax((double)bufferData[smp], envelope * alpha);
            bufferData[smp] = (float)envelope;
        }
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
        alpha = exp(-1.0 / n);
    }
    
    double release;
    double sampleRate = 1.0;
    double alpha = 0.0;
    double envelope = 0.0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReleaseFilter)
};

