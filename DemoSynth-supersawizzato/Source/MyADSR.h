/*
  ==============================================================================

    MyADSR.h
    Created: 8 May 2025 3:43:38pm
    Author:  Derin Donmez

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "PluginParameters.h"

class MyADSR
{
public:
    MyADSR(float defaultAtk = Parameters::defaultAtk, float defaultDcy = Parameters::defaultDcy, float defaultSus = Parameters::defaultSus, float defaultRel = Parameters::defaultRel):ampAdsrParams(defaultAtk, defaultDcy, defaultSus, defaultRel){}
    ~MyADSR(){}
    
    void noteOn()
    {
        adsr1.noteOn();
        adsr2.noteOn();
    }
    
    void noteOff()
    {
        adsr1.noteOff();
        adsr2.noteOff();
    }
    
    void applyEnvelopeToBuffer (AudioBuffer<float>& buffer, const int startSample, const int numSamples)
    {
        adsr1.applyEnvelopeToBuffer(buffer, startSample, numSamples);
        adsr2.applyEnvelopeToBuffer(buffer, startSample, numSamples);
    }
    
    bool isActive()
    {
        return (adsr1.isActive() && adsr2.isActive());
    }
    
    void prepareToPlay (const double newSampleRate)
    {
        adsr1.setSampleRate(newSampleRate);
        adsr2.setSampleRate(newSampleRate);
        setParameters(ampAdsrParams);
    }
    
    void setParameters (const ADSR::Parameters& newParameters)
    {
        adsr1.setParameters(newParameters);
        adsr2.setParameters(newParameters);
    }
    
    void setAttack(const float newValue)
    {
        ampAdsrParams.attack = newValue;
        setParameters(ampAdsrParams);
    }

    void setDecay(const float newValue)
    {
        ampAdsrParams.decay = newValue;
        setParameters(ampAdsrParams);
    }

    void setSustain(const float newValue)
    {
        ampAdsrParams.sustain = sqrt(newValue);
        setParameters(ampAdsrParams);
    }

    void setRelease(const float newValue)
    {
        ampAdsrParams.release = newValue;
        setParameters(ampAdsrParams);
    }
    
    float getNextSample()
    {
        return adsr1.getNextSample() * adsr2.getNextSample();
    }
    
private:
    ADSR adsr1;
    ADSR adsr2;
    ADSR::Parameters ampAdsrParams;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MyADSR)
};
