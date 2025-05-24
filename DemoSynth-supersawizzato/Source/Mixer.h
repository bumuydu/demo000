/*
  ==============================================================================

    Mixer.h
    Created: 23 May 2025 4:40:35pm
    Author:  Derin Donmez

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class Mixer {
public:
    Mixer(float defaultSaw = 1.0f, float defaultSub = 0.0f, float defaultNoise = 0.0f) :
        sawGain(defaultSaw), subGain(defaultSub), noiseGain(defaultNoise)
    {
        masterGain.setCurrentAndTargetValue(Decibels::decibelsToGain(0.0f));
//        sawGainn.setCurrentAndTargetValue(Decibels::decibelsToGain(-2.0f));
    }
    ~Mixer(){};
    
    void prepareToPlay(const int sampleRate)
    {
        masterGain.reset(sampleRate, 0.001f);
//        sawGainn.reset(sampleRate, 0.01f);
    }
    
//    mixer.getNextAudioBlock(mixerBuffer, oscillatorBuffer, subBuffer, noiseBuffer, startSample, numSamples, sawOscs.getActiveOscs());
    void getNextAudioBlock(AudioBuffer<float>& mixerBuffer, AudioBuffer<float>& oscillatorBuffer, AudioBuffer<float>& subBuffer, AudioBuffer<float>& noiseBuffer, const int startSample, const int numSamples, const float velocity, const int activeOscs)
    {
        // Volume proporzionale alla velocity
        for (int ch = 0; ch < 2; ++ch)
        {
            oscillatorBuffer.applyGain(ch, startSample, numSamples, velocity * sawGain / std::sqrt(activeOscs));
//            sawGainn.applyGain(oscillatorBuffer.getWritePointer(ch) + startSample, numSamples);
        }
        
        subBuffer.applyGain(startSample, numSamples, velocity * subGain);
        
        // mix all buffers into one
        for (int ch = 0; ch < 2; ++ch)
        {
            mixerBuffer.addFrom(ch, startSample, oscillatorBuffer, ch, startSample, numSamples);
            mixerBuffer.addFrom(ch, startSample, subBuffer, 0, startSample, numSamples);
            mixerBuffer.addFrom(ch, startSample, noiseBuffer, 0, startSample, numSamples);
        }
    }
    
    void applyMasterGainAndCopy(AudioBuffer<float>& outputBuffer, AudioBuffer<float>& mixerBuffer,
                                const int startSample, const int numSamples)
    {
        for (int ch = 0; ch < 2; ++ch)
        {
            masterGain.applyGain(mixerBuffer.getWritePointer(ch) + startSample, numSamples);
        }
        
        for (int ch = 0; ch < 2; ++ch)
        {
            outputBuffer.addFrom(ch, startSample, mixerBuffer, ch, startSample, numSamples);
        }
    }
    
    void setSawGain(const float newValue)
    {
        sawGain = newValue;
        
//        sawGainn.setTargetValue(newValue);
//        updateGain();
    }
     
    void setSubGain(const float newValue)
    {
        subGain = newValue;
    }
    
    void setNoiseGain(const float newValue)
    {
        noiseGain = newValue;
    }
    
    const float getNoiseGain()
    {
        return noiseGain;
    }
    
    void setMasterGain(const float newValue)
    {
        masterGain.setTargetValue(Decibels::decibelsToGain(newValue));
    }
    
//    void updateGain(const float velocity, const int activeOscs)
//    {
//        sawGainn.setTargetValue(sawGain * velocity / std::sqrt(activeOscs()));
//    }
    
private:
    SmoothedValue<float, ValueSmoothingTypes::Linear> masterGain;

    float sawGain;
    float subGain;
    float noiseGain;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Mixer)
};
