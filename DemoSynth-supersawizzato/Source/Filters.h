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

