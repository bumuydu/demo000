#pragma once
#include <JuceHeader.h>
#include "Synth.h"
#include "PluginParameters.h"
#include "PolySynth.h"

class DemoSynthAudioProcessor  : public juce::AudioProcessor, public AudioProcessorValueTreeState::Listener
{
public:
    DemoSynthAudioProcessor();
    ~DemoSynthAudioProcessor() override {}

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

//    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override  { return JucePlugin_Name; }

    bool acceptsMidi() const override;
    bool producesMidi() const override           { return false; }
    bool isMidiEffect() const override           { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    int getNumPrograms() override                { return 1; }
    int getCurrentProgram() override             { return 0; }
    void setCurrentProgram(int index) override   {};
    const juce::String getProgramName (int index) override { return {}; }
    void changeProgramName (int index, const juce::String& newName) override {}
    AudioPlayHead::CurrentPositionInfo retriveAudioPositionInfo(AudioPlayHead* plyHead);
    
    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
//    void panic();

private:
    void parameterChanged(const String& paramID, float newValue) override;

    AudioProcessorValueTreeState parameters;
//    Synthesiser mySynth;
    PolySynthesiser mySynth;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DemoSynthAudioProcessor)
};
