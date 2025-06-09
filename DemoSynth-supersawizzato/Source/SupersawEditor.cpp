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
    setupKnob(mainWaveformSlider,      75,  47, 85, 85);
    setupKnob(mainRegisterSlider,     215,  47, 85, 85);
    setupKnob(numSawsSlider,           75, 147, 85, 85);
    setupKnob(detuneSlider,           215, 147, 85, 85);
    setupKnob(phaseSlider,             75, 247, 85, 85);
    setupKnob(stereoWidthSlider,      215, 247, 85, 85);
    setupToggle(phaseResettingToggle,  40, 290, 30, 30);
    setupKnob(subRegSlider,            75, 400, 85, 85);
    setupKnob(subWaveformSlider,      215, 400, 85, 85);
    setupSlider(sawLevelSlider,       380,  70, 60, 260);
    setupSlider(subLevelSlider,       450,  70, 60, 260);
    setupSlider(noiseLevelSlider,     520,  70, 60, 260);
    setupKnob(noiseReleaseSlider,     385, 400, 85, 85);
    setupKnob(noiseColorSlider,       490, 400, 85, 85);
    setupKnob(cutoffSlider,           640,  60, 120, 120);
    setupKnob(qualitySlider,          660, 190, 85, 85);
    setupKnob(lfoAmtSlider,           660, 295, 85, 85);
    setupKnob(egAmtSlider,            660, 400, 85, 85);
    setupKnob(lfoWaveformSlider,      830,  70, 85, 85);
    setupKnob(lfoFreqSlider,          830, 180, 85, 85);
    setupKnob(lfoRateSlider,          830, 180, 85, 85);
    setupToggle(lfoSyncToggle,        800, 215, 30, 30);
    setupSlider(attackSlider,         980,  70, 45, 200);
    setupSlider(decaySlider,         1035,  70, 45, 200);
    setupSlider(sustainSlider,       1090,  70, 45, 200);
    setupSlider(releaseSlider,       1145,  70, 45, 200);
    setupKnob(masterSlider,          1115, 320, 85, 85);
    setupKnob(oversamplingSlider,    1100, 390, 80, 30);

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
    
    loadWaveIcons();
    
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
        g.fillAll(juce::Colour(0xFF303b4b));
    }
    
    // border
    {
        g.setColour(juce::Colour(0xFFc67856));
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(5.0f), 10.0f, 3.0f);
    }
    
    // panels
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
    
    // panel labels
    {
        g.setFont(juce::Font("Futura", 16.0f, juce::Font::bold));
        g.setColour(juce::Colour(0xFFc67856));
        
        // to the left
        g.drawText("MAIN OSC", 30, 25, 150, 20, juce::Justification::left);
        g.drawText("SUB", 30, 365, 80, 20, juce::Justification::left);
        g.drawText("MIXER", 370, 25, 100, 20, juce::Justification::left);
        g.drawText("NOISE", 370, 365, 80, 20, juce::Justification::left);
        g.drawText("FILTER", 630, 25, 100, 20, juce::Justification::left);
        g.drawText("LFO", 810, 25, 100, 20, juce::Justification::left);
        g.drawText("ENV", 970, 25, 100, 20, juce::Justification::left);
//        g.drawText("MASTER", 1120, 300, 100, 20, juce::Justification::left);
        
        // centered
//        g.drawText("MAIN OSC", 20, 25, 320, 20, juce::Justification::centred);
//        g.drawText("SUB",      20, 365, 320, 20, juce::Justification::centred);
//        g.drawText("MIXER",   360, 25, 240, 20, juce::Justification::centred);
//        g.drawText("NOISE",   360, 365, 240, 20, juce::Justification::centred);
//        g.drawText("FILTER",  620, 25, 160, 20, juce::Justification::centred);
//        g.drawText("LFO",     800, 25, 140, 20, juce::Justification::centred);
//        g.drawText("ENV",     960, 25, 250, 20, juce::Justification::centred);
////        g.drawText("MASTER", 1100, 300, 110, 20, juce::Justification::centred);
    }
    
    // param labels
    {
        g.setFont(12.0f);
//        g.setFont(juce::Font::bold);
        g.setFont(juce::Font("Lato", 13.0f, juce::Font::plain));
        g.setColour(juce::Colour(0xFFf0f1f1));
        g.drawFittedText("Waveform",         75,  27, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("Register",         215, 27, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("# of Saws",        75, 127, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("Detune",           215, 127, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("Phase",            75, 227, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("Stereo Width",     215, 227, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("Phase Reset",       40, 270, 30, 20, juce::Justification::centred, 1);
        g.drawFittedText("Sub Reg",           75, 380, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("Sub Waveform",     215, 380, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("Saw",             380,  50, 60, 20, juce::Justification::centred, 1);
        g.drawFittedText("Sub",             450,  50, 60, 20, juce::Justification::centred, 1);
        g.drawFittedText("Noise",           520,  50, 60, 20, juce::Justification::centred, 1);
        g.drawFittedText("Noise Release",   385, 380, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("Noise Color",     490, 380, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("Cutoff",          640,  45, 120, 20, juce::Justification::centred, 1);
        g.drawFittedText("Quality",         660, 170, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("LFO Amt",         660, 275, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("EG Amt",          660, 380, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("LFO Waveform",    830,  50, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("LFO Freq/Rate",   830, 160, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("Sync",            800, 195, 30, 20, juce::Justification::centred, 1);
        g.drawFittedText("Attack",          980,  50, 45, 20, juce::Justification::centred, 1);
        g.drawFittedText("Decay",          1035,  50, 45, 20, juce::Justification::centred, 1);
        g.drawFittedText("Sustain",        1090,  50, 45, 20, juce::Justification::centred, 1);
        g.drawFittedText("Release",        1145,  50, 45, 20, juce::Justification::centred, 1);
        g.drawFittedText("Master",         1115, 300, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("Oversampling",   1100, 370, 80, 20, juce::Justification::centred, 1);
//        g.drawText("MASTER", 1120, 300, 100, 20, juce::Justification::left);
    }
    
    // main waveforms
//    setupKnob(mainWaveformSlider,      x=75,  y=47, w=85, h=85);
    const float cx = 75+42;
    const float cy = 47+42;
    const float radius = 85 * 0.5f;
    const float iconSize = 5.0f;

    for (int i = 0; i < 7; ++i)
    {
        if (waveIcons[i].isValid())
        {
            
            const float angle = juce::MathConstants<float>::pi * (5.0f / 4.0f - i * (1.5f / 6.0f));
            const float iconX = cx + radius * std::cos(angle) - iconSize * 0.5f;
            const float iconY = cy + radius * std::sin(angle) - iconSize * 0.5f;

            g.drawImage(waveIcons[i], iconX, iconY, iconSize, iconSize,
                        0, 0, waveIcons[i].getWidth(), waveIcons[i].getHeight());
        }
        else
            DBG("icons not valid");
    }
    
    // title
    {
        g.setFont(juce::Font("Futura", 24.0f, juce::Font::bold));
        g.setColour(juce::Colours::white);
        g.drawFittedText("Super-super", getWidth() - 330, getHeight() - 60, 300, 40, juce::Justification::centredRight, 1);
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
//    slider.setTextBoxStyle(Slider::TextBoxBelow, false, 60, 15);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setPopupDisplayEnabled(true, true, this);
    addAndMakeVisible(&slider);

    slider.setBounds(x, y, w, h);
}

void SupersawEditor::setupSlider(Slider& slider, int x, int y, int w, int h)
{
    slider.setSliderStyle(Slider::SliderStyle::LinearBarVertical);
//    slider.setTextBoxStyle(Slider::TextBoxBelow, false, 60, 15);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setPopupDisplayEnabled(true, true, this);
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

//void SupersawEditor::loadWaveIcons()
//{
//    for (int i = 0; i < 7; ++i)
//    {
//        auto imageStream = juce::File::getCurrentWorkingDirectory()
//                           .getChildFile("Resources")
//                           .getChildFile("images")
//                           .getChildFile(juce::String(i + 1) + ".png")
//                           .createInputStream();
//
//        if (imageStream != nullptr)
//            waveIcons[i] = juce::PNGImageFormat().decodeImage(*imageStream);
//    }
//}

void SupersawEditor::loadWaveIcons()
{
    auto baseDir = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                       .getParentDirectory()
                       .getChildFile("Resources")
                       .getChildFile("images");

    for (int i = 0; i < 7; ++i)
    {
        auto imageFile = baseDir.getChildFile(juce::String(i + 1) + ".png");

        if (imageFile.existsAsFile())
        {
            std::unique_ptr<juce::InputStream> stream(imageFile.createInputStream());

            if (stream != nullptr)
            {
                auto img = juce::PNGImageFormat().decodeImage(*stream);

                if (img.isValid())
                    waveIcons[i] = img;
                else
                    DBG("Failed to decode image: " + imageFile.getFullPathName());
            }
            else
            {
                DBG("Failed to create input stream for: " + imageFile.getFullPathName());
            }
        }
        else
        {
            DBG("Image file does not exist: " + imageFile.getFullPathName());
        }
    }
}
