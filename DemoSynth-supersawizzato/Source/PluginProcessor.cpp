#include "PluginProcessor.h"

// Polifonia del mio sintetizzatore (numero di voci)
#define NUM_VOICES 8

//==============================================================================
DemoSynthAudioProcessor::DemoSynthAudioProcessor()
     : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)), // Niente input
       parameters(*this, nullptr, "SynthSettings", { Parameters::createParameterLayout() })
{
    // Per prima cosa aggiungo uno o più SynthSound al synth
    mySynth.addSound(new MySynthSound());

    // Poi aggiungo tante voci di polifonia quante voglio che possa gestirne
    for (int v = 0; v < NUM_VOICES; ++v)
        mySynth.addVoice(new SimpleSynthVoice(Parameters::defaultAtk, Parameters::defaultDcy, Parameters::defaultSus, Parameters::defaultRel));

    // E poi attacco il processor ai parametri, come al solito
    Parameters::addListenerToAllParameters(parameters, this);
}

//==============================================================================

// Questo è l'unico metodo diverso rispetto a un normale plugin VST
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

// Supporta mono o stereo out (input non presente, come definito nel costruttore)
bool DemoSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void DemoSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    const auto numSamples = buffer.getNumSamples();

    // Pulisco il buffer (non c'è input, e le SynthVoice sommano, non sovrascrivono)
    buffer.clear();

    // Lascio che la classe Synthsiser faccia le sue magie
    mySynth.renderNextBlock(buffer, midiMessages, 0, numSamples);

    // Se l'output è stereo copio sul secondo canale il contenuto del primo
//    if (getTotalNumOutputChannels() == 2)
//        buffer.copyFrom(1, 0, buffer, 0, 0, numSamples);
    
    // modify: for mono output --> do we accept or is it stereo by default?
    //  if mono is available: do we change the logic on how we use the buffer.addFrom() ?
}

//==============================================================================

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
    // Il cambio di parametri va propagato a tutte le voci di polifonia del synth
    for (int v = 0; v < mySynth.getNumVoices(); ++v)
        if (auto voice = dynamic_cast<SimpleSynthVoice*>(mySynth.getVoice(v)))
        {
            // OSC
            if (paramID == Parameters::nameSawReg)
                voice->setSawRegister(newValue);
            
            if (paramID == Parameters::nameSawNum)
                voice->setSawNum(newValue);
            
            if (paramID == Parameters::nameDetune)
                voice->setSawDetune(newValue);
            
            if (paramID == Parameters::nameStereoWidth)
                voice->setSawStereoWidth(newValue);
            
            if (paramID == Parameters::namePhase)
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
            
            if (paramID == Parameters::nameSubReg)
                voice->setSubReg(roundToInt(newValue));
            
            if (paramID == Parameters::nameSubWf)
                voice->setSubWf(roundToInt(newValue));
            
            if (paramID == Parameters::nameFiltHz)
                voice->setCutoff(newValue);
            
            if (paramID == Parameters::nameFiltQ)
                voice->setQuality(newValue);
            
            if (paramID == Parameters::nameNRel)
                voice->setNoiseRelease(newValue);
            
            if (paramID == Parameters::nameNFilt)
                voice->setNoiseFilterCutoff(newValue);
                        
//            if (paramID == Parameters::nameFiltEnv)
//                voice->setFilterEnvAmt(newValue);
//            
//            if (paramID == Parameters::nameFiltLfo)
//                voice->setFilterLfoAmt(newValue);

                
        }

}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DemoSynthAudioProcessor();
}
