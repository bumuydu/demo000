#pragma once
#include <JuceHeader.h>
#include "Oscillators.h"
#include "Filters.h"

class MySynthSound : public SynthesiserSound
{
public:
	MySynthSound() {}
	~MySynthSound() {}

	bool appliesToNote(int midiNoteNumber) override { return true; }
	bool appliesToChannel(int midiChannel) override { return true; }

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MySynthSound)
};


class SimpleSynthVoice : public SynthesiserVoice
{
public:
	SimpleSynthVoice(int defaultSawReg = 0, int defaultSawNum = 5, int defaultDetune = 15, float defaultPhase = 0.0f, float defaultStereoWidth = 0.0f, float defaultAtk = 0.005f, float defaultDcy = 0.025f, float defaultSus = 0.6f, float defaultRel = 0.7f, float defaultSaw = 1.0f, float defaultSub = 0.0f, float defaultNoise = 0.0f,  int defaultSubReg = 0 /*, int defaultSubWf = 0*/)
    : sawOscs(defaultSawReg, defaultSawNum, defaultDetune, defaultPhase, defaultStereoWidth), ampAdsrParams(defaultAtk, defaultDcy, defaultSus, defaultRel),  sawGain(defaultSaw), subGain(defaultSub), noiseGain(defaultNoise), sawRegister(defaultSawReg), subRegister(defaultSubReg)/*, subWaveform(defaultSubWf)*/
	{
	};
	
	~SimpleSynthVoice() {};

	// ==== Metodi astratti ereditati da SynthesiserVoice che devono essere implementati ====

	// Deve ritornare true se questa voce è adibita alla riproduzione di un certo SynthSound.
	bool canPlaySound(SynthesiserSound* sound) override
	{
		// Se la classe Synthesiser di JUCE sta cercando di suonare un "MySynthSound" con questa voce
		// allora ritorna true, altrimenti il dynamic casting fallisce (= nullptr) e ritorna false.
		// (per questo esempio non servono meccanismi più raffinati)
		return dynamic_cast<MySynthSound*>(sound) != nullptr;
	}
    
    void releaseResources()
    {
        noiseOsc.releaseResources();
        oscillatorBuffer.setSize(0, 0);
        subBuffer.setSize(0, 0);
        noiseBuffer.setSize(0, 0);
        mixerBuffer.setSize(0, 0);
    }

	// Metodo chiamato dalla classe Synthesiser per ogni Note-on assegnato a questa voce
	void startNote(int midiNoteNumber, float velocity, SynthesiserSound* sound, int currentPitchWheelPosition) override
	{	
		// Reset phase for each oscillator
        subOscillator.reset();
        sawOscs.startNote(midiNoteNumber);
        
        // storing currentMidiNote for parameter changes related to the frequency
        currentMidiNote = midiNoteNumber;
        float baseFreq = MidiMessage::getMidiNoteInHertz(midiNoteNumber) * std::pow(2, sawRegister);
        sawOscs.setSawFreqs(baseFreq);
        
        // sub frequency calculation
            // +1 because the default value is 0, but sub is supposed to be 1 oct below main osc
        float subFreq = baseFreq / std::pow(2, subRegister + 1);
        subOscillator.setFrequency(subFreq);

		// Triggero l'ADSR
		ampAdsr.noteOn();
        
		// Mi salvo la velocity da usare come volume della nota suonata
		velocityLevel = velocity;
        trigger = true;
	}
	
	// Metodo chiamato dalla classe Synthesiser per ogni Note-off assegnato a questa voce
	void stopNote(float velocity, bool allowTailOff) override
	{
		// Triggero la fase di release dell'ADSR
		ampAdsr.noteOff();

		// Se mi chiede di non generare code (o altri casi, tipo se l'ADSR ha già finito la fase di release)
		// allora segnalo alla classe Synthesiser che questa voce è libera per poter riprodurre altri suoni
        if (!allowTailOff || ( !ampAdsr.isActive() /*&& noiseOsc.envFinished()*/))
			clearCurrentNote();
	}
	
	// Metodo chiamato dalla classe Synthesiser per ogni Control Change ricevuto
	void controllerMoved(int controllerNumber, int newControllerValue) override {}
	
	// Metodo chiamato dalla classe Synthesiser in caso venga mossa la pitch-wheel
	void pitchWheelMoved(int newPitchWheelValue) override {}

	// Metodo chiamato da Synthesiser per chiedere alla voce di riempire l'outputBuffer.
	// Noterete che non c'è il buffer MIDI, questo perchè in caso di eventi MIDI all'interno di
	// un buffer di lunghezza normale, la classe Synthsiser spezza il buffer in più blocchi:
	// MIDI events:   !.......!....!...!.........!.....
	// AudioBuffers:  |.....|.|....|...|.....|...|.....
	// In questo modo si può renderizzare l'audio senza preoccuparsi degli eventi MIDI (che 
	// sono gestiti attraverso gli altri metodi precedenti).
	// ATTENZIONE: Questo è realizzato non istanziando diversi mini-buffer, ma passando il buffer
	// intero insieme a due interi: startSample e numSamples, tenete sempre conto di questi due valori
	// quando andate a riempire l'output buffer!
	// Si noti che lo stesso buffer viene passato a tutte le voci di polifonia, non va quindi riempito
	// sovrascrivendo il contenuto, ma gli va sommato sopra quanto generato nella SynthVoice.
	//
	void renderNextBlock(AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
	{

		// [Solitamente qui ci stanno cose tipo gli LFO]

		// Se la voce non è attiva ci si può fermare qui.
		// In caso si voglia comunque "far trascorrere il tempo" per alcune componenti (ad esempio 
		// far avanzare gli LFO anche mentre la voce non sta suonando), queste operazioni vanno
		// fatte prima di questo controllo.
		if (!isVoiceActive())
			return;

		// Pulizia del buffer di lavoro
		// (userò solo i primi "numSamples" del buffer, da sommare poi nel
		// buffer di output, da startSample, per numSamples campioni)
		oscillatorBuffer.clear(0, numSamples);
        subBuffer.clear(0, numSamples);
        noiseBuffer.clear(0, numSamples);
        mixerBuffer.clear(0, numSamples);

		// Preparazione del ProcessContext per le classi DSP
		auto voiceData = oscillatorBuffer.getArrayOfWritePointers();
        
        // modify: must change the number of channels here to 2 to make it stereo
		dsp::AudioBlock<float> audioBlock{ voiceData, 2, (size_t)numSamples };
		dsp::ProcessContextReplacing<float> context{ audioBlock };
        
        auto subData = subBuffer.getArrayOfWritePointers();
        dsp::AudioBlock<float> subAudioBlock{ subData, 1, (size_t)numSamples };
        dsp::ProcessContextReplacing<float> subContext{ subAudioBlock };

        auto mixerData = mixerBuffer.getArrayOfWritePointers();
        dsp::AudioBlock<float> mixerBlock{ mixerData, 2, (size_t)numSamples };
//        dsp::ProcessContextReplacing<float> mixerContext{ mixerBlock };
        
		// Genero la mie forme d'onda        
        sawOscs.process(context);
        subOscillator.process(subContext);
        
        // noise: trigger the ReleaseFilter envelope
        if(trigger)
        {
            noiseOsc.trigger(startSample, velocityLevel);
            trigger = false;
        }
        
        // and process the noise
        noiseOsc.process(noiseBuffer, startSample, numSamples, noiseGain);
        
		// [Solitamente qui ci stanno cose tipo mixer degli oscillatori, filtro e saturazione]
        
        // filtering the noise with its parameter before the main LPF
        noiseFilter.processBlock(noiseBuffer, numSamples);
        
		// La modulo in ampiezza con l'ADSR
		ampAdsr.applyEnvelopeToBuffer(oscillatorBuffer, 0, numSamples);
        ampAdsr.applyEnvelopeToBuffer(subBuffer, 0, numSamples);

		// Volume proporzionale alla velocity
        oscillatorBuffer.applyGain(0, numSamples, velocityLevel * sawGain / sqrt(sawOscs.getActiveOscs()));
//        for (int ch = 0; ch < oscillatorBuffer.getNumChannels(); ++ch)
//        {
//            oscillatorBuffer.applyGain(ch, 0, numSamples, velocityLevel * sawGain / std::sqrt(sawOscs.getActiveOscs()));
//        }
        subBuffer.applyGain(0, numSamples, velocityLevel * subGain);
//        mixerBuffer.applyGain(0, numSamples, 1);
        
        // mix all buffers into one
        mixerBuffer.addFrom(0, 0, oscillatorBuffer, 0, 0, numSamples);
        mixerBuffer.addFrom(1, 0, oscillatorBuffer, 1, 0, numSamples);

        // Then: add mono sub and noise to *both* channels
        for (int ch = 0; ch < 2; ++ch)
        {
            mixerBuffer.addFrom(ch, 0, subBuffer, 0, 0, numSamples);
            mixerBuffer.addFrom(ch, 0, noiseBuffer, 0, 0, numSamples);
        }
        
        // process the mixed buffer through a ladder filter
//        ladderFilter.process(mixerBuffer);
        for (int ch = 0; ch < 2; ++ch)
        {
            auto chBlock = mixerBlock.getSingleChannelBlock(ch);
            dsp::ProcessContextReplacing<float> chContext(chBlock);
            ladderFilters[ch].process(chContext);
//            ladderFilters[ch].process(mixerContext);
        }
        
		// copy the filtered buffer to the output buffer
        for (int ch = 0; ch < 2; ++ch)
            outputBuffer.addFrom(ch, startSample, mixerBuffer, ch, 0, numSamples);
        
		// Se gli ADSR hanno finito la fase di decay (o se ho altri motivi per farlo)
		// segno la voce come libera per suonare altre note
        if (!ampAdsr.isActive() && noiseOsc.envFinished())
        {
            clearCurrentNote();
            trigger = false;
        }
		
	}

	// ==== Metodi personali ====

	// Prepare to play of a single voice
	void prepareToPlay(double sampleRate, int samplesPerBlock)
	{
		// Resetto gli ADSR
		ampAdsr.setSampleRate(sampleRate);
		ampAdsr.setParameters(ampAdsrParams);

		// Preparo le ProcessSpecs per l'oscillatore ed eventuali altre classi DSP
		// MODIFY: spec for sawOscs might be different --> oversampling
        spec.maximumBlockSize = samplesPerBlock;
		spec.sampleRate = sampleRate;
		spec.numChannels = 1;
        stereoSpec.maximumBlockSize = samplesPerBlock;
        stereoSpec.sampleRate = sampleRate;
        stereoSpec.numChannels = 2;
        
		// Inizializzo l'oscillatore
        // modify: send oversampled sampleRate etc. to the sawOscs
        sawOscs.prepareToPlay(stereoSpec);
        subOscillator.prepare(spec);
        for (int ch = 0; ch < 2; ++ch)
        {
            ladderFilters[ch].prepare(spec);
        }

        noiseOsc.prepareToPlay(spec);
        noiseFilter.prepareToPlay(spec);

		// Se non ho intenzione di generare un segnale intrinsecamente
		// stereo è inutile calcolare più di un canale. Ne calcolo 1 e 
		// poi nel PluginProcessor lo copio su tutti i canali in uscita
        // WILL MODIFY THIS PART!
		oscillatorBuffer.setSize(2, samplesPerBlock);
        subBuffer.setSize(1, samplesPerBlock);
        noiseBuffer.setSize(1, samplesPerBlock);
        mixerBuffer.setSize(2, samplesPerBlock);
	}
    
    void updateFreqs()
    {
//        if (isVoiceActive()) {
            float baseFreq = MidiMessage::getMidiNoteInHertz(currentMidiNote) * std::pow(2, sawRegister);
            sawOscs.setSawFreqs(baseFreq);
            
            float subFreq = baseFreq / std::pow(2, subRegister + 1);
            subOscillator.setFrequency(subFreq);
//        }
    }
	
    // Parameter setters
    void setSawRegister(const int newValue)
    {
        // -2 because the newValue is the index of the AudioParameterChoice which isn't the actual value
        sawRegister = newValue - 2;
        sawOscs.setRegister(sawRegister);
        updateFreqs();
    }
    
    void setSawNum(const int newValue)
    {
        sawOscs.setActiveOscs(newValue);
        //activeOscs = newValue;
    }
    
    void setSawDetune(const float newValue)
    {
        sawOscs.setDetune(newValue);
    }
    
    void setSawStereoWidth(const float newValue)
    {
        sawOscs.setStereoWidth(newValue);
    }
    
    void setSawPhase(const float newValue)
    {
        sawOscs.setPhase(newValue);
    }
    
    void setSawGain(const float newValue)
    {
        sawGain = newValue; // modify: make it smooth
    }
    
    void setSubGain(const float newValue)
    {
        subGain = newValue; // modify: smooth it out
    }
    
    void setNoiseGain(const float newValue)
    {
        noiseGain = newValue;   // modify: smooth it out
    }
    
	// Setters of JUCE's ADSR parameters -- MODIFY: Add slope param. A,D,R ^slope , S^(1/slope)
	void setAttack(const float newValue)
	{
		ampAdsrParams.attack = newValue;
		ampAdsr.setParameters(ampAdsrParams);
	}

	void setDecay(const float newValue)
	{
		ampAdsrParams.decay = newValue;
		ampAdsr.setParameters(ampAdsrParams);
	}

	void setSustain(const float newValue)
	{
		ampAdsrParams.sustain = newValue;
		ampAdsr.setParameters(ampAdsrParams);
	}

	void setRelease(const float newValue)
	{
		ampAdsrParams.release = newValue;
		ampAdsr.setParameters(ampAdsrParams);
	}
    
    void setSubReg(const int newValue)
    {
        subRegister = newValue;
        updateFreqs();
    }

    void setSubWf(const int newValue)
    {
        switch (newValue)
                {
                case 0: // sinusoidale
                    subOscillator = dsp::Oscillator<float> { [](float x) {return std::sin(x); } };
                    break;
                case 1: // square
                    subOscillator = dsp::Oscillator<float> { [](float x)
                        {
                            return x > 0.0f ? 1.0f : -1.0f;
                        } };
                    break;
                default:
                    subOscillator = dsp::Oscillator<float> { [](float x) {return std::sin(x); } };
                    // info: Sub Oscillator not selected correctly
                    //jassertfalse;
                    break;
                }
        //dsp::ProcessSpec spec;
        subOscillator.prepare(spec);
        updateFreqs();
    }
    
    void setCutoff(const float newValue)
    {
        for (int ch = 0; ch < 2; ++ch)
        {
            ladderFilters[ch].setCutoffFrequencyHz(newValue);
        }
//        ladderFilter.setCutoff(newValue);
    }
    
    void setQuality(const float newValue)
    {
        for (int ch = 0; ch < 2; ++ch)
        {
            ladderFilters[ch].setResonance(newValue);
        }
//        ladderFilter.setResonance(newValue);
    }
    
//    void setFilterEnvAmt(const float newValue)
//    {
//        ladderFilter.setEnvAmt(newValue);
//    }
//    
//    void nameFiltLfo(const float newValue)
//    {
//        ladderFilter.setLfoAmt(newValue);
//    }
    
    void setNoiseFilterCutoff(const float newValue)
    {
        noiseFilter.setFrequency(newValue);
    }
    
    void setNoiseRelease(const float newValue)
    {
        noiseOsc.setRelease(newValue);
    }
    
private:
	// La classe dsp::Oscillator può essere inizializzata con una lambda da usare come forma d'onda
	// (x va da -pi a + pi) e con un intero facoltativo che (se presente e diverso da 0) indica alla
	// classe di usare una lookup table di quelle dimensioni.
	// Se si vuole usare un oscillatore dalla forma d'onda variabile o con più parametri è meglio
	// implementarne uno come quello visto a lezione.
    
    dsp::ProcessSpec spec;
    dsp::ProcessSpec stereoSpec;

    SawOscillators sawOscs;
    NoiseOsc noiseOsc;
    
    // Sub Oscillator
    dsp::Oscillator<float> subOscillator{ [](float x) {return std::sin(x); } };

    // to track detune, register parameters on active note
    int currentMidiNote = 60;

	// Linear ADSR --> modify: will add a param (slope) which will make the lines nice and exponential curves
	ADSR ampAdsr;
	ADSR::Parameters ampAdsrParams;
    
    // used for triggering the noise envelope
    bool trigger = false;
    
    // osc params
    int sawRegister;
    
    // osc level params --> MODIFY: might make a mixer class
    float sawGain;
    float subGain;
    float noiseGain;
    
    // sub params
    int subRegister;
    
    // modify: are these necessary?
    //int subWaveform;
    
    // filters
    LadderFilter ladderFilters[2];                              // LPF for the whole synth
    StereoFilter noiseFilter;                              // filter for noise
    
    // buffers
	AudioBuffer<float> oscillatorBuffer;
    AudioBuffer<float> subBuffer;
    AudioBuffer<float> noiseBuffer;
    AudioBuffer<float> mixerBuffer;
	float velocityLevel = 0.7f;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleSynthVoice)
};
