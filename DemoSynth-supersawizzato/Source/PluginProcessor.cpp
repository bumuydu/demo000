#include "PluginProcessor.h"
#include "PluginParameters.h"
#include "SupersawEditor.h"

#define NUM_VOICES 8

//==============================================================================
DemoSynthAudioProcessor::DemoSynthAudioProcessor()
     : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
       parameters(*this, nullptr, "SynthSettings", { Parameters::createParameterLayout() })
{
    mySynth.addSound(new MySynthSound());

    for (int v = 0; v < NUM_VOICES; ++v)
        mySynth.addVoice(new SimpleSynthVoice(Parameters::defaultAtk, Parameters::defaultDcy, Parameters::defaultSus, Parameters::defaultRel));

    Parameters::addListenerToAllParameters(parameters, this);
}

//==============================================================================

bool DemoSynthAudioProcessor::acceptsMidi() const
{
    return true;
}

//==============================================================================
void DemoSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mySynth.setCurrentPlaybackSampleRate(sampleRate);
    mySynth.setNoteStealingEnabled(true);

    for (int v = 0; v < mySynth.getNumVoices(); ++v)
        if (auto voice = dynamic_cast<SimpleSynthVoice*>(mySynth.getVoice(v)))
            voice->prepareToPlay(sampleRate, samplesPerBlock);
}

void DemoSynthAudioProcessor::releaseResources()
{
    for (int v = 0; v < mySynth.getNumVoices(); ++v)
        if (auto voice = dynamic_cast<SimpleSynthVoice*>(mySynth.getVoice(v)))
            voice->releaseResources();
}

bool DemoSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void DemoSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    const auto numSamples = buffer.getNumSamples();
    AudioPlayHead::CurrentPositionInfo hostPosition = retriveAudioPositionInfo(getPlayHead());
    for (int v = 0; v < mySynth.getNumVoices(); ++v)
        if (auto voice = dynamic_cast<SimpleSynthVoice*>(mySynth.getVoice(v)))
            voice->updatePosition(hostPosition);
    
    buffer.clear();

    mySynth.renderNextBlock(buffer, midiMessages, 0, numSamples);
}

bool DemoSynthAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* DemoSynthAudioProcessor::createEditor()
{
    return new SupersawEditor(*this, parameters);
}

//==============================================================================

    AudioPlayHead::CurrentPositionInfo DemoSynthAudioProcessor::retriveAudioPositionInfo(AudioPlayHead* plyHead)
    {
    AudioPlayHead::CurrentPositionInfo hostPosition;

    // getCurrentPosition DEPRECATED
    if (plyHead == nullptr || !plyHead->getCurrentPosition(hostPosition))
    {
        hostPosition.bpm = 120.0;
        hostPosition.isPlaying = false;
        hostPosition.ppqPosition = 0.0;
        hostPosition.ppqPositionOfLastBarStart = 0.0;
        hostPosition.timeSigDenominator = 4;
        hostPosition.timeSigNumerator = 4;
        hostPosition.timeInSamples = 0;
        hostPosition.timeInSeconds = 0.0;
    }

    return hostPosition;
}

void DemoSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void DemoSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(ValueTree::fromXml(*xmlState));
}

void DemoSynthAudioProcessor::parameterChanged(const String& paramID, float newValue)
{
    for (int v = 0; v < mySynth.getNumVoices(); ++v)
        if (auto voice = dynamic_cast<SimpleSynthVoice*>(mySynth.getVoice(v)))
        {
            // OSC
            if (paramID == Parameters::nameMainWf)
                voice->setMainWf(newValue);
            
            if (paramID == Parameters::nameSawReg)
                voice->setSawRegister(newValue);
            
            if (paramID == Parameters::nameSawNum)
                voice->setSawNum(newValue);
            
            if (paramID == Parameters::nameDetune)
                voice->setSawDetune(newValue);
            
            if (paramID == Parameters::nameStereoWidth)
                voice->setSawStereoWidth(newValue);
            
            if (paramID == Parameters::namePhase)
                voice->setPhaseResetting(newValue);
            
            if (paramID == Parameters::namePhaseDegree)
                voice->setSawPhase(newValue);
            
            // OSC levels
            if (paramID == Parameters::nameSawLev)
                voice->setSawGain(Decibels::decibelsToGain(newValue, Parameters::dbFloor));

            if (paramID == Parameters::nameSubLev)
                voice->setSubGain(Decibels::decibelsToGain(newValue, Parameters::dbFloor));
            
            if (paramID == Parameters::nameNLev)
                voice->setNoiseGain(Decibels::decibelsToGain(newValue, Parameters::dbFloor));
            
            // ADSR
            if (paramID == Parameters::nameAtk)
                voice->setAttack(newValue);

            if (paramID == Parameters::nameDcy)
                voice->setDecay(newValue);

            if (paramID == Parameters::nameSus)
                voice->setSustain(newValue);

            if (paramID == Parameters::nameRel)
                voice->setRelease(newValue);
            
            // SUB
            if (paramID == Parameters::nameSubReg)
                voice->setSubReg(roundToInt(newValue));
            
            if (paramID == Parameters::nameSubWf)
                voice->setSubWf(roundToInt(newValue));
            
            // FILTER
            if (paramID == Parameters::nameFiltHz)
                voice->setCutoff(newValue);
            
            if (paramID == Parameters::nameFiltQ)
                voice->setQuality(newValue);
            
            if (paramID == Parameters::nameNRel)
                voice->setNoiseRelease(newValue);
            
            if (paramID == Parameters::nameNFilt)
                voice->setNoiseFilterCutoff(newValue);
                        
            if (paramID == Parameters::nameFiltEnv)
                voice->setFilterEnvAmt(newValue);
            
            // FILTER & LFO
            if (paramID == Parameters::nameFiltLfoAmt)
                voice->setFilterLfoAmt(newValue);
            
            if (paramID == Parameters::nameLfoWf)
                voice->setLfoWf(newValue);
            
            if (paramID == Parameters::nameLfoFreq)
                voice->setLfoFreq(newValue);
            
            if (paramID == Parameters::nameLfoRate)
                voice->setLfoRate(newValue);
            
            if (paramID == Parameters::nameLfoSync)
                voice->setLfoSync(newValue);
            
            if (paramID == Parameters::nameOversampling)
                voice->setOversampling(newValue);
            
            if (paramID == Parameters::nameMaster)
                voice->setMasterGain(newValue);
        }

}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DemoSynthAudioProcessor();
}
