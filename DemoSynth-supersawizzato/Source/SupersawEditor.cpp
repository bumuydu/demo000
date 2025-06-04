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
//    setupSlider(mainWaveformSlider, x, y, w, h);
//    setupSlider(mainRegisterSlider, x, y, w, h);
//    setupSlider(numSawsSlider, x, y, w, h);
//    setupSlider(detuneSlider, x, y, w, h);
//    setupSlider(stereoWidthSlider, x, y, w, h);
//    setupSlider(phaseResettingSlider, x, y, w, h);
//    setupSlider(phaseSlider, x, y, w, h);
//    setupSlider(subRegSlider, x, y, w, h);
//    setupSlider(subWaveformSlider, x, y, w, h);
//    setupSlider(sawLevelSlider, x, y, w, h);
//    setupSlider(subLevelSlider, x, y, w, h);
//    setupSlider(noiseLevelSlider, x, y, w, h);
//    setupSlider(cutoffSlider, x, y, w, h);
//    setupSlider(qualitySlider, x, y, w, h);
//    setupSlider(egAmtSlider, x, y, w, h);
//    setupSlider(lfoWaveformSlider, x, y, w, h);
//    setupSlider(lfoAmtSlider, x, y, w, h);
//    setupSlider(lfoFreqSlider, x, y, w, h);
//    setupSlider(lfoRateSlider, x, y, w, h);
//    setupSlider(lfoSyncSlider, x, y, w, h);
//    setupSlider(attackSlider, x, y, w, h);
//    setupSlider(decaySlider, x, y, w, h);
//    setupSlider(sustainSlider, x, y, w, h);
//    setupSlider(releaseSlider, x, y, w, h);
//    setupSlider(noiseReleaseSlider, x, y, w, h);
//    setupSlider(noiseColorSlider, x, y, w, h);
//    setupSlider(oversamplingSlider, x, y, w, h);
//    setupSlider(masterSlider, x, y, w, h);

//    mainWfAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameMainWf, ));
//    mainRegAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameSawReg, ));
//    numSawsAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameSawNum, ));
//    detuneAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameDetune, ));
//    stereoWidthAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameStereoWidth, ));
//    phaseResettingAtttachmen.reset(new SliderAttachment(valueTreeState, Parameters::namePhase, ))t
//    phaseAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::namePhaseDegree, ));
//    subRegAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameSubReg, ));
//    subWfAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameSubWf, ));
//    sawLevelAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameSawLev, ));
//    subLevelAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameSubLev, ));
//    noiseLevelAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameNLev, ));
//    cutoffAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameFiltHz, ));
//    qualityAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameFiltQ, ));
//    egAmtAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameFiltEnv, ));
//    lfoWfAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameLfoWf, ));
//    lfoAmtAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameFiltLfoAmt, ));
//    lfoFreqAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameLfoFreq, ));
//    lfoRateAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameLfoRate, ));
//    lfoSyncAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameLfoSync, ));
//    attackAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameAtk, ));
//    decayAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameDcy, ));
//    sustainAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameSus, ));
//    releaseAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameRel, ));
//    noiseReleaseAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameNRel, ));
//    noiseColorAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameNFilt, ));
//    oversamplingAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameOversampling, ));
//    masterAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameMaster, ));
    
    
//    timeAtttachment.reset(new SliderAttachment(valueTreeState, Parameters::nameDelayTime, timeSlider));

    //shapeSlider.setLookAndFeel(&myTheme);
    this->setLookAndFeel(&supersawTheme);

    setSize (600, 400);
}

SupersawEditor::~SupersawEditor()
{
    //if we had used the GUI editor we would have put the .reset() here to destroy the Attachments before the sliders
    //shapeSlider.setLookAndFeel(nullptr);
    this->setLookAndFeel(nullptr);
}

//==============================================================================
void SupersawEditor::paint (juce::Graphics& g)
{
    //we create these scopes to let our compiler know what to keep close and what to forget
    {
        //0x to enter hexcodes in c++, ff to have the background fully opaque (alpha), then rgb (ARGB)
//        g.setGradientFill(ColourGradient(Colour(0xff484848), 270, 25, Colour(0xff303030), 300, 385, false));
//        g.fillAll();
    }

    {
//        g.setColour(Colours::white);
//        // we set the normal Juce font but at 15p
//        g.setFont(Font(15.0f));
//        g.drawText("Time", 78, 59, 74, 21, Justification::centred);
//        g.drawText("Feedback", 264, 59, 74, 21, Justification::centred);
//        g.drawText("Dry/Wet", 446, 59, 74, 21, Justification::centred);
//        g.drawText("Mod Amot", 78, 219, 74, 21, Justification::centred);
//        g.drawText("Mod Freq", 264, 219, 74, 21, Justification::centred);
//        g.drawText("Mod Shape", 446, 219, 74, 21, Justification::centred);
    }
}

void SupersawEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

void SupersawEditor::setupSlider(Slider& slider, int x, int y, int w, int h)
{
    slider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    slider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(&slider);

    slider.setBounds(x, y, w, h);
}
