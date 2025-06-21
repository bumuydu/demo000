/*
  ==============================================================================

    SupersawEditor.h
    Created: 24 May 2025 1:51:11pm
    Author:  Derin Donmez

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SupersawTheme.h"

typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

class SupersawEditor  : public juce::AudioProcessorEditor
{
public:
    SupersawEditor (DemoSynthAudioProcessor&, AudioProcessorValueTreeState&);
    ~SupersawEditor() override;
   
    void paint (juce::Graphics&) override;
    void resized() override;

private:

    void setupKnob(Slider& slider, int x, int y, int w, int h);
    void setupSlider(Slider& slider, int x, int y, int w, int h);
    void setupHorizontalSlider(Slider& slider, int x, int y, int w, int h);
    void setupToggle(ToggleButton& button, int x, int y, int w, int h);
    void loadWaveIcons();
    void drawWaveforms(Graphics& g);
    
    Image waveIcons[7];
    Image waveIcons2[9];
    
    DemoSynthAudioProcessor& audioProcessor;
    AudioProcessorValueTreeState& valueTreeState;
        
    Slider mainWaveformSlider;
    Slider mainRegisterSlider;
    Slider numSawsSlider;
    Slider detuneSlider;
    Slider stereoWidthSlider;
    ToggleButton phaseResettingToggle;
    Slider subRegSlider;
    Slider subWaveformSlider;
    Slider sawLevelSlider;
    Slider subLevelSlider;
    Slider noiseLevelSlider;
    Slider cutoffSlider;
    Slider qualitySlider;
    Slider egAmtSlider;
    Slider lfoWaveformSlider;
    Slider lfoAmtSlider;
    Slider lfoFreqSlider;
    Slider lfoRateSlider;
    ToggleButton lfoSyncToggle;
    Slider attackSlider;
    Slider decaySlider;
    Slider sustainSlider;
    Slider releaseSlider;
    Slider noiseReleaseSlider;
    Slider noiseColorSlider;
    Slider masterSlider;    
    
    SupersawLookAndFeel supersawTheme;

    std::unique_ptr<SliderAttachment> mainWfAtttachment;
    std::unique_ptr<SliderAttachment> mainRegAtttachment;
    std::unique_ptr<SliderAttachment> numSawsAtttachment;
    std::unique_ptr<SliderAttachment> detuneAtttachment;
    std::unique_ptr<SliderAttachment> stereoWidthAtttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> phaseResettingAtttachment;
    std::unique_ptr<SliderAttachment> subRegAtttachment;
    std::unique_ptr<SliderAttachment> subWfAtttachment;
    std::unique_ptr<SliderAttachment> sawLevelAtttachment;
    std::unique_ptr<SliderAttachment> subLevelAtttachment;
    std::unique_ptr<SliderAttachment> noiseLevelAtttachment;
    std::unique_ptr<SliderAttachment> cutoffAtttachment;
    std::unique_ptr<SliderAttachment> qualityAtttachment;
    std::unique_ptr<SliderAttachment> egAmtAtttachment;
    std::unique_ptr<SliderAttachment> lfoWfAtttachment;
    std::unique_ptr<SliderAttachment> lfoAmtAtttachment;
    std::unique_ptr<SliderAttachment> lfoFreqAtttachment;
    std::unique_ptr<SliderAttachment> lfoRateAtttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lfoSyncAttachment;
    std::unique_ptr<SliderAttachment> attackAtttachment;
    std::unique_ptr<SliderAttachment> decayAtttachment;
    std::unique_ptr<SliderAttachment> sustainAtttachment;
    std::unique_ptr<SliderAttachment> releaseAtttachment;
    std::unique_ptr<SliderAttachment> noiseReleaseAtttachment;
    std::unique_ptr<SliderAttachment> noiseColorAtttachment;
    std::unique_ptr<SliderAttachment> masterAtttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SupersawEditor)
};
