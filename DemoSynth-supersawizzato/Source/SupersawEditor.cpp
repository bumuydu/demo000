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
    mainWaveformSlider.setName("wf");
    lfoWaveformSlider.setName("wf2");
    
    setupKnob(mainWaveformSlider,      77,  54, 75, 75);
    setupKnob(mainRegisterSlider,     217,  58, 75, 75);
    setupKnob(numSawsSlider,           77, 158, 75, 75);
    setupKnob(detuneSlider,           217, 158, 75, 75);
    setupKnob(phaseSlider,             77, 254, 75, 75);
    setupKnob(stereoWidthSlider,      217, 254, 75, 75);
    setupToggle(phaseResettingToggle,  45, 292, 30, 30);
    setupKnob(subRegSlider,            77, 408, 75, 75);
    setupSlider(subWaveformSlider,    205, 408, 85, 85);
    setupSlider(sawLevelSlider,       380,  70, 60, 260);
    setupSlider(subLevelSlider,       450,  70, 60, 260);
    setupSlider(noiseLevelSlider,     520,  70, 60, 260);
    setupKnob(noiseReleaseSlider,     385, 408, 75, 75);
    setupKnob(noiseColorSlider,       490, 408, 75, 75);
    setupKnob(cutoffSlider,           640,  60, 120, 120);
    setupKnob(qualitySlider,          662, 198, 75, 75);
    setupKnob(lfoAmtSlider,           662, 303, 75, 75);
    setupKnob(egAmtSlider,            662, 408, 75, 75);
    setupKnob(lfoWaveformSlider,      832,  70, 75, 75);
    setupKnob(lfoFreqSlider,          832, 188, 75, 75);
    setupKnob(lfoRateSlider,          832, 188, 75, 75);
    setupToggle(lfoSyncToggle,        802, 228, 30, 30);
    setupSlider(attackSlider,         980,  70, 45, 200);
    setupSlider(decaySlider,         1035,  70, 45, 200);
    setupSlider(sustainSlider,       1090,  70, 45, 200);
    setupSlider(releaseSlider,       1145,  70, 45, 200);
    setupKnob(masterSlider,          1112, 328, 75, 75);
    setupSlider(oversamplingSlider,  1110, 400, 80, 70);
    
    // hide bpm synchronised lfo rate
    lfoRateSlider.setVisible(false);
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
    
    subWaveformSlider.setSliderStyle(Slider::LinearHorizontal);
    subWaveformSlider.setRange(0, 1, 1);  // only values 0 or 1
//    subWaveformSlider.setSnapToMousePosition(true); // snaps immediately to nearest value
    oversamplingSlider.setSliderStyle(Slider::LinearHorizontal);
    oversamplingSlider.setRange(0, 1, 1);  // only values 0 or 1
//    oversamplingSlider.setSnapToMousePosition(true); // snaps immediately to nearest value
    
    lfoSyncToggle.onClick = [this]()
    {
        const bool syncOn = lfoSyncToggle.getToggleState();

        lfoRateSlider.setVisible(syncOn);
        lfoFreqSlider.setVisible(!syncOn);
    };
    
    loadWaveIcons();
    loadWaveIcons2();
    
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
    drawPanel({1090, 300, 120, 155}, juce::Colour(0xff2b2d31)); // MASTER
    
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
//        g.drawFittedText("Waveform",         72,  42, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("Register",        212, 37, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("# of Oscs",       72, 138, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("Detune",          212, 138, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("Phase",           72, 234, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("Stereo Width",    212, 234, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("Phase",           45, 259, 30, 20, juce::Justification::centred, 1);
        g.drawFittedText("Reset",           45, 272, 30, 20, juce::Justification::centred, 1);
        g.drawFittedText("Register",        72, 388, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("Waveform",        208, 388, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("Saw",             380,  50, 60, 20, juce::Justification::centred, 1);
        g.drawFittedText("Sub",             450,  50, 60, 20, juce::Justification::centred, 1);
        g.drawFittedText("Noise",           520,  50, 60, 20, juce::Justification::centred, 1);
        g.drawFittedText("Release",         382, 388, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("Color",           487, 388, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("Cutoff",          640,  45, 120, 20, juce::Justification::centred, 1);
        g.drawFittedText("Quality",         657, 178, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("LFO Amt",         657, 283, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("EG Amt",          657, 388, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("Waveform",        827,  45, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("Freq/Rate",       827, 168, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("BPM",             802, 195, 30, 20, juce::Justification::centred, 1);
        g.drawFittedText("Sync",            802, 208, 30, 20, juce::Justification::centred, 1);
        g.drawFittedText("Attack",          980,  45, 45, 20, juce::Justification::centred, 1);
        g.drawFittedText("Decay",          1035,  45, 45, 20, juce::Justification::centred, 1);
        g.drawFittedText("Sustain",        1090,  45, 45, 20, juce::Justification::centred, 1);
        g.drawFittedText("Release",        1145,  45, 45, 20, juce::Justification::centred, 1);
        g.drawFittedText("Master",         1107, 308, 85, 20, juce::Justification::centred, 1);
        g.drawFittedText("Oversampling",   1110, 400, 80, 20, juce::Justification::centred, 1);
    }
    
    // draw waveform details
    drawWaveforms(g);
    
    
    // title
    {
        g.setFont(juce::Font("Futura", 24.0f, juce::Font::bold));
        g.setColour(juce::Colours::white);
        g.drawFittedText("Supercore", getWidth() - 330, getHeight() - 50, 300, 40, juce::Justification::centredRight, 1);
    }
    {
        // author
        g.setFont(juce::Font("Lato", 14.0f, juce::Font::plain));
        g.setColour(juce::Colours::lightgrey);
        g.drawText("Coded at LIM by Derin Donmez", getWidth() - 330, getHeight() - 60, 300, 20, juce::Justification::centredRight);
    }
}

void SupersawEditor::resized()
{
}

void SupersawEditor::setupKnob(Slider& slider, int x, int y, int w, int h)
{
    slider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setPopupDisplayEnabled(true, true, this);
    addAndMakeVisible(&slider);

    slider.setBounds(x, y, w, h);
}

void SupersawEditor::setupSlider(Slider& slider, int x, int y, int w, int h)
{
    slider.setSliderStyle(Slider::SliderStyle::LinearBarVertical);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setPopupDisplayEnabled(true, true, this);
    addAndMakeVisible(&slider);

    slider.setBounds(x, y, w, h);
}

void SupersawEditor::setupToggle(ToggleButton& button, int x, int y, int w, int h)
{
    addAndMakeVisible(button);
    button.setBounds(x, y, w, h);
}

void SupersawEditor::loadWaveIcons()
{
    for (int i = 0; i < 7; ++i)
    {
        juce::Image img;

        switch (i)
        {
            case 0: img = juce::ImageCache::getFromMemory(BinaryData::_1_png, BinaryData::_1_pngSize); break;
            case 1: img = juce::ImageCache::getFromMemory(BinaryData::_2_png, BinaryData::_2_pngSize); break;
            case 2: img = juce::ImageCache::getFromMemory(BinaryData::_3_png, BinaryData::_3_pngSize); break;
            case 3: img = juce::ImageCache::getFromMemory(BinaryData::_4_png, BinaryData::_4_pngSize); break;
            case 4: img = juce::ImageCache::getFromMemory(BinaryData::_5_png, BinaryData::_5_pngSize); break;
            case 5: img = juce::ImageCache::getFromMemory(BinaryData::_6_png, BinaryData::_6_pngSize); break;
            case 6: img = juce::ImageCache::getFromMemory(BinaryData::_7_png, BinaryData::_7_pngSize); break;
        }

        if (img.isValid())
        {
            waveIcons[i] = img;
        }
        else
        {
            DBG("Failed to load wave icon image " + juce::String(i + 1));
        }
    }
}

void SupersawEditor::loadWaveIcons2()
{
    juce::StringArray filenames = { "sine.png", "3.png", "4.png", "5.png", "env1.png", "env2.png", "cutoff.png", "cutoff2.png" };

    for (int i = 0; i < filenames.size(); ++i)
    {
        juce::Image img;

        if      (filenames[i] == "sine.png")    img = juce::ImageCache::getFromMemory(BinaryData::sine_png, BinaryData::sine_pngSize);
        else if (filenames[i] == "3.png")       img = juce::ImageCache::getFromMemory(BinaryData::_3_png, BinaryData::_3_pngSize);
        else if (filenames[i] == "4.png")       img = juce::ImageCache::getFromMemory(BinaryData::_4_png, BinaryData::_4_pngSize);
        else if (filenames[i] == "5.png")       img = juce::ImageCache::getFromMemory(BinaryData::_5_png, BinaryData::_5_pngSize);
        else if (filenames[i] == "env1.png")    img = juce::ImageCache::getFromMemory(BinaryData::env1_png, BinaryData::env1_pngSize);
        else if (filenames[i] == "env2.png")    img = juce::ImageCache::getFromMemory(BinaryData::env2_png, BinaryData::env2_pngSize);
        else if (filenames[i] == "cutoff.png")  img = juce::ImageCache::getFromMemory(BinaryData::cutoff_png, BinaryData::cutoff_pngSize);
        else if (filenames[i] == "cutoff2.png") img = juce::ImageCache::getFromMemory(BinaryData::cutoff2_png, BinaryData::cutoff2_pngSize);

        if (img.isValid())
        {
            waveIcons2[i] = img;
        }
        else
        {
            DBG("Failed to load BinaryData image: " + filenames[i]);
        }
    }
}

void SupersawEditor::drawWaveforms(Graphics& g)
{
    const float cx = 77.0f + 75.0f * 0.5f;
    const float cy = 54.0f + 75.0f * 0.5f;
    const float radius = 75 * 0.5f;
    const float iconSize = 13.0f;

    for (int i = 0; i < 7; ++i)
    {
        if (waveIcons[i].isValid())
        {
            // start at bottom-left (135°), end at bottom-right (45°), moving clockwise
            float startAngle = juce::MathConstants<float>::pi * 0.70f;
            float endAngle = juce::MathConstants<float>::pi * 2.30f;
            const float angleStep  = (endAngle - startAngle) / 6.0f; // 6 steps between 7 points

            const float angle = startAngle + i * angleStep;

            // --- tick position
            const float tickDistance = radius - 8.0f;
            const float tickLength = 4.0f;

            const float tickX = cx + tickDistance * std::cos(angle);
            const float tickY = cy + tickDistance * std::sin(angle);

            const float tickEndX = tickX + tickLength * std::cos(angle);
            const float tickEndY = tickY + tickLength * std::sin(angle);

            g.setColour(juce::Colour(0xFFf0f1f1));
            g.drawLine(tickX, tickY, tickEndX, tickEndY, 1.8f);

            const float iconDistance = radius * 1.20f;
            const float iconX = cx + iconDistance * std::cos(angle) - iconSize * 0.5f;
            const float iconY = cy + iconDistance * std::sin(angle) - iconSize * 0.5f;


            g.drawImage(waveIcons[i], iconX, iconY, iconSize, iconSize,
                        0, 0, waveIcons[i].getWidth(), waveIcons[i].getHeight());
        }
        else
        {
            DBG("icons not valid");
        }
    }

    // lfo waveforms
    const float cx2 = 832.0f + 75.0f * 0.5f;
    const float cy2 = 70.0f + 75.0f * 0.5f;
    const float radius2 = 75 * 0.5f;
    const float iconSize2 = 13.0f;

    // Custom order: sine, 3, 4, 5
    // Assuming waveIcons2 is an array of 4 juce::Image objects in this order
    for (int i = 0; i < 4; ++i)
    {
        if (waveIcons2[i].isValid())
        {
            // Adjust angles for 4 points spread nicely around the knob, e.g. from 135° to 45°
            // Let's use the same arc but split into 3 steps between 4 points:
            float startAngle2 = juce::MathConstants<float>::pi * 0.70f;  // ~126°
            float endAngle2 = juce::MathConstants<float>::pi * 2.30f;    // ~414°
            const float angleStep2 = (endAngle2 - startAngle2) / 3.0f;  // 3 steps for 4 points

            const float angle2 = startAngle2 + i * angleStep2;

            // Tick position
            const float tickDistance2 = radius2 - 8.0f;
            const float tickLength2 = 4.0f;

            const float tickX2 = cx2 + tickDistance2 * std::cos(angle2);
            const float tickY2 = cy2 + tickDistance2 * std::sin(angle2);

            const float tickEndX2 = tickX2 + tickLength2 * std::cos(angle2);
            const float tickEndY2 = tickY2 + tickLength2 * std::sin(angle2);

            g.setColour(juce::Colour(0xFFf0f1f1));
            g.drawLine(tickX2, tickY2, tickEndX2, tickEndY2, 1.8f);

            // Icon position
            const float iconDistance2 = radius2 * 1.20f;
            const float iconX2 = cx2 + iconDistance2 * std::cos(angle2) - iconSize2 * 0.5f;
            const float iconY2 = cy2 + iconDistance2 * std::sin(angle2) - iconSize2 * 0.5f;


            g.drawImage(waveIcons2[i], iconX2, iconY2, iconSize2, iconSize2,
                        0, 0, waveIcons2[i].getWidth(), waveIcons2[i].getHeight());
        }
        else
        {
            DBG("waveIcons2[" << i << "] not valid");
        }
    }
    
    // sub waveforms
    g.drawImage(waveIcons2[0], 218, 413, 20.0f, 18.0f, 0, 0, waveIcons2[0].getWidth(), waveIcons2[0].getHeight());
    g.drawImage(waveIcons[5], 258, 413, 20.0f, 18.0f, 0, 0, waveIcons[5].getWidth(), waveIcons[5].getHeight());
    
    // envelopes for the filter egAmtSlider: 660, 400, 85, 85);
    g.drawImage(waveIcons2[5], 645, 460, 20.0f, 20.0f, 0, 0, waveIcons2[5].getWidth(), waveIcons2[5].getHeight());
    g.drawImage(waveIcons2[4], 730, 460, 20.0f, 20.0f, 0, 0, waveIcons2[4].getWidth(), waveIcons2[4].getHeight());
    
    // noise cutoff image       490, 400, 85, 85)
    g.drawImage(waveIcons2[6], 470, 465, 25.0f, 12.5f, 0, 0, waveIcons2[6].getWidth(), waveIcons2[6].getHeight());
    g.drawImage(waveIcons2[7], 560, 465, 25.0f, 12.5f, 0, 0, waveIcons2[7].getWidth(), waveIcons2[7].getHeight());
}
