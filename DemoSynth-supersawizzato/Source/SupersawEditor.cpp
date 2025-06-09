/*
  ==============================================================================

    SupersawEditor.cpp
    Created: 24 May 2025 1:51:05pm
    Author:  Derin Donmez

  ==============================================================================
*/

#include "SupersawEditor.h"

#include "PluginProcessor.h"
#include "SupersawEditor.h"
#include "PluginParameters.h"

//==============================================================================
SupersawEditor::SupersawEditor (DemoSynthAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{
    setupKnob(mainWaveformSlider, 70,  40, 95, 95);
    setupKnob(mainRegisterSlider, 210,  40, 95, 95);
    setupKnob(numSawsSlider, 70, 140, 95, 95);
    setupKnob(detuneSlider, 210, 140, 95, 95);
    setupKnob(phaseSlider, 70, 240, 95, 95);
    setupKnob(stereoWidthSlider, 210, 240, 95, 95);
    setupToggle(phaseResettingToggle, 50, 280, 30, 30);
    setupKnob(subRegSlider, 70, 390, 95, 95);
    setupKnob(subWaveformSlider, 210, 390, 95, 95);
    setupSlider(sawLevelSlider, 380,  60, 60, 260);
    setupSlider(subLevelSlider, 450,  60, 60, 260);
    setupSlider(noiseLevelSlider, 520,  60, 60, 260);
    setupKnob(noiseReleaseSlider, 380, 390, 95, 95);
    setupKnob(noiseColorSlider, 485, 390, 95, 95);
    setupKnob(cutoffSlider, 640,  50, 120, 120);
    setupKnob(qualitySlider, 655, 180, 95, 95);
    setupKnob(lfoAmtSlider, 655, 285, 95, 95);
    setupKnob(egAmtSlider, 655, 390, 95, 95);
    setupKnob(lfoWaveformSlider, 825, 50, 95, 95);
    setupKnob(lfoFreqSlider, 825, 160, 95, 95);
    setupKnob(lfoRateSlider, 825, 160, 95, 95);
    setupToggle(lfoSyncToggle, 805, 200, 30, 30);
    setupSlider(attackSlider, 980,  60, 45, 200);
    setupSlider(decaySlider, 1035,  60, 45, 200);
    setupSlider(sustainSlider, 1090,  60, 45, 200);
    setupSlider(releaseSlider, 1145,  60, 45, 200);
    setupKnob(masterSlider, 1110, 310, 90, 90);
    setupKnob(oversamplingSlider, 1100, 380, 80, 30);
//    setupHorizontalSlider(oversamplingSlider, 1120, 380, 80, 30);
    
    // hide bpm synchronised lfo rate
    lfoRateSlider.setVisible(false);
//    lfoSyncSlider.setbut
    lfoSyncToggle.setButtonText("Sync");
    phaseResettingToggle.setButtonText("On/Off");

    mainWfAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameMainWf, mainWaveformSlider));
    mainRegAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameSawReg, mainRegisterSlider));
    numSawsAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameSawNum, numSawsSlider));
    detuneAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameDetune, detuneSlider));
    stereoWidthAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameStereoWidth, stereoWidthSlider));
    phaseResettingAtttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, Parameters::namePhase, phaseResettingToggle));
    phaseAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::namePhaseDegree, phaseSlider));
    subRegAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameSubReg, subRegSlider));
    subWfAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameSubWf, subWaveformSlider));
    sawLevelAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameSawLev, sawLevelSlider));
    subLevelAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameSubLev, subLevelSlider));
    noiseLevelAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameNLev, noiseLevelSlider));
    cutoffAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameFiltHz, cutoffSlider));
    qualityAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameFiltQ, qualitySlider));
    egAmtAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameFiltEnv, egAmtSlider));
    lfoWfAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameLfoWf, lfoWaveformSlider));
    lfoAmtAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameFiltLfoAmt, lfoAmtSlider));
    lfoFreqAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameLfoFreq, lfoFreqSlider));
    lfoRateAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameLfoRate, lfoRateSlider));
    lfoSyncAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, Parameters::nameLfoSync, lfoSyncToggle));
    attackAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameAtk, attackSlider));
    decayAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameDcy, decaySlider));
    sustainAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameSus, sustainSlider));
    releaseAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameRel, releaseSlider));
    noiseReleaseAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameNRel, noiseReleaseSlider));
    noiseColorAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameNFilt, noiseColorSlider));
    oversamplingAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameOversampling, oversamplingSlider));
    masterAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameMaster, masterSlider));
    
    lfoSyncToggle.onClick = [this]()
    {
        const bool syncOn = lfoSyncToggle.getToggleState();

        lfoRateSlider.setVisible(syncOn);
        lfoFreqSlider.setVisible(!syncOn);
    };
    
    this->setLookAndFeel(&supersawTheme);

    setSize (1240, 520);
}

SupersawEditor::~SupersawEditor()
{
    //if we had used the GUI editor we would have put the .reset() here to destroy the Attachments before the sliders
    this->setLookAndFeel(nullptr);
}

//==============================================================================
void SupersawEditor::paint (juce::Graphics& g)
{
    {
        // background
        g.fillAll(juce::Colour(0xFF56627e));
    }
    
    // border
    {
        g.setColour(juce::Colours::orange);
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(5.0f), 10.0f, 3.0f);
    }
    
    const auto drawPanel = [&](juce::Rectangle<int> bounds, juce::Colour colour)
    {
        g.setColour(colour);
        g.fillRoundedRectangle(bounds.toFloat(), 10.0f);
    };
    
    drawPanel({20, 20, 320, 320}, juce::Colour(0xFF1C2531)); // MAIN OSC
    drawPanel({20, 360, 320, 140}, juce::Colour(0xFF1C2531)); // SUB
    drawPanel({360, 20, 240, 320}, juce::Colour(0xFF1C2531)); // MIXER
    drawPanel({360, 360, 240, 140}, juce::Colour(0xFF1C2531)); // NOISE
    drawPanel({620, 20, 160, 480}, juce::Colour(0xFF1C2531)); // FILTER
    drawPanel({800, 20, 140, 260}, juce::Colour(0xFF1C2531)); // LFO
    drawPanel({960, 20, 250, 260}, juce::Colour(0xFF1C2531)); // ENV
    drawPanel({1100, 300, 110, 110}, juce::Colour(0xff2b2d31)); // MASTER
    
    // labels
    {
        g.setFont(16.0f);
        g.setColour(juce::Colours::orange);
        g.drawText("MAIN OSC", 30, 25, 150, 20, juce::Justification::left);
        g.drawText("SUB", 30, 365, 80, 20, juce::Justification::left);
        g.drawText("MIXER", 370, 25, 100, 20, juce::Justification::left);
        g.drawText("NOISE", 370, 365, 80, 20, juce::Justification::left);
        g.drawText("FILTER", 630, 25, 100, 20, juce::Justification::left);
        g.drawText("LFO", 810, 25, 100, 20, juce::Justification::left);
        g.drawText("ENV", 970, 25, 100, 20, juce::Justification::left);
//        g.drawText("MASTER", 1120, 300, 100, 20, juce::Justification::left);
    }
    
    // title
    {
        g.setFont(juce::Font("Georgia", 24.0f, juce::Font::bold));
        g.setColour(juce::Colours::white);
        g.drawFittedText("KODUMU OTURTTUM", getWidth() - 330, getHeight() - 60, 300, 40, juce::Justification::centredRight, 1);
    }
    {
        // author
        g.setFont(14.0f);
        g.setColour(juce::Colours::lightgrey);
        g.drawText("Coded at LIM by Derin Donmez", getWidth() - 330, getHeight() - 80, 300, 20, juce::Justification::centredRight);
    }
}

void SupersawEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

void SupersawEditor::setupKnob(Slider& slider, int x, int y, int w, int h)
{
    slider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    slider.setTextBoxStyle(Slider::TextBoxBelow, false, 60, 15);
    addAndMakeVisible(&slider);

    slider.setBounds(x, y, w, h);
}

void SupersawEditor::setupSlider(Slider& slider, int x, int y, int w, int h)
{
    slider.setSliderStyle(Slider::SliderStyle::LinearBarVertical);
    slider.setTextBoxStyle(Slider::TextBoxBelow, false, 60, 15);
    addAndMakeVisible(&slider);

    slider.setBounds(x, y, w, h);
}

void SupersawEditor::setupToggle(ToggleButton& button, int x, int y, int w, int h)
{
    addAndMakeVisible(button);
    button.setBounds(x, y, w, h);
//    button.setButtonText("Sync");
}

//void SupersawEditor::setupHorizontalSlider(Slider& slider, int x, int y, int w, int h)
//{
//    slider.setSliderStyle(Slider::SliderStyle::LinearHorizontal);
//    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 80, 20);
//    addAndMakeVisible(&slider);
//
//    slider.setBounds(x, y, w, h);
//}
