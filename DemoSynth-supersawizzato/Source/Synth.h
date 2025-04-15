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
	SimpleSynthVoice(int defaultSawReg = 0, int defaultSawNum = 5, int defaultDetune = 15, float defaultPhase = 0.0f, float defaultStereoWidth = 0.0f, float defaultAtk = 0.005f, float defaultDcy = 0.025f, float defaultSus = 0.6f, float defaultRel = 0.7f, float defaultNoiseRel = 0.25f, float defaultSaw = 1.0f, float defaultSub = 0.0f, float defaultNoise = 0.0f,  int defaultSubReg = 0 /*, int defaultSubWf = 0, int defaultNoiseFilter = 0.5f*/)
    : sawOscs(defaultSawReg, defaultSawNum, defaultDetune, defaultPhase, defaultStereoWidth), ampAdsrParams(defaultAtk, defaultDcy, defaultSus, defaultRel), egNoise(defaultNoiseRel), sawGain(defaultSaw), subGain(defaultSub), noiseGain(defaultNoise), sawRegister(defaultSawReg), subRegister(defaultSubReg)/*, subWaveform(defaultSubWf),  noiseFiltParam(defaultNoiseFilter)*/
	{
        noiseFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        noiseFilter.setCutoffFrequency(20000.0f);
        noiseFilter.setResonance(0.1f);
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
        noiseEnvelope.setSize(0, 0);
//        oscillators.releaseResources();
    }

	// Metodo chiamato dalla classe Synthesiser per ogni Note-on assegnato a questa voce
	void startNote(int midiNoteNumber, float velocity, SynthesiserSound* sound, int currentPitchWheelPosition) override
	{	
		// Reset phase for each oscillator
        subOscillator.reset();
        noiseFilter.reset();
        sawOscs.startNote(midiNoteNumber, velocity);
        
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
        if (!allowTailOff || ( !ampAdsr.isActive()))
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
        noiseEnvelope.clear(0, numSamples);
        
        //DBG("Noise Gain: " << noiseGain);

		// Preparazione del ProcessContext per le classi DSP
		auto voiceData = oscillatorBuffer.getArrayOfWritePointers();
        
        // modify: must change the number of channels here to 2 to make it stereo
		dsp::AudioBlock<float> audioBlock{ voiceData, 1, (size_t)numSamples };
		dsp::ProcessContextReplacing<float> context{ audioBlock };
        
        auto subData = subBuffer.getArrayOfWritePointers();
        dsp::AudioBlock<float> subAudioBlock{ subData, 1, (size_t)numSamples };
        dsp::ProcessContextReplacing<float> subContext{ subAudioBlock };
        
        auto noiseData = noiseBuffer.getArrayOfWritePointers();

        auto mixerData = mixerBuffer.getArrayOfWritePointers();
        dsp::AudioBlock<float> mixerBlock{ mixerData, 1, (size_t)numSamples };
        dsp::ProcessContextReplacing<float> mixerContext{ mixerBlock };
        
		// Genero la mie forme d'onda
        sawOscs.process(context);
        subOscillator.process(subContext);
        
        // noise oscillator with the ReleaseFilter
        if(trigger)
        {
            noiseEnvelope.setSample(0, startSample, velocityLevel);
            trigger = false;
        }
        
        egNoise.processBlock(noiseEnvelope, startSample, numSamples);
        // modify: realistically, this will only be calculated once and then added to both channels of the outputBuffer
        const auto numChannels = noiseBuffer.getNumChannels();
        for (int ch = 0; ch < numChannels; ++ch)
            for (int smp = 0; smp < numSamples; ++smp)
                noiseData[ch][smp] += velocityLevel * noiseGain * (noise.nextFloat() * 2.0f) - 1.0f;
        
		// [Solitamente qui ci stanno cose tipo mixer degli oscillatori, filtro e saturazione]
        
        // filtering the noise with its parameter before the main LPF
//        noiseFilter.process(noiseContext); // old method with Oscillator instance
        noiseFilter2.processBlock(noiseBuffer, numSamples);
        
        for (int ch = 0; ch < numChannels; ++ch)
        {
            FloatVectorOperations::multiply(noiseData[ch], noiseEnvelope.getReadPointer(0), numSamples);
        }
        
		// La modulo in ampiezza con l'ADSR
		ampAdsr.applyEnvelopeToBuffer(oscillatorBuffer, 0, numSamples);
        ampAdsr.applyEnvelopeToBuffer(subBuffer, 0, numSamples);

		// Volume proporzionale alla velocity
        oscillatorBuffer.applyGain(0, numSamples, velocityLevel * sawGain / sqrt(sawOscs.getActiveOscs()));
        subBuffer.applyGain(0, numSamples, velocityLevel * subGain);
        mixerBuffer.applyGain(0, numSamples, 1);
        
        // mix all buffers into one
        mixerBuffer.addFrom(0, 0, oscillatorBuffer, 0, 0, numSamples);
        mixerBuffer.addFrom(0, 0, subBuffer, 0, 0, numSamples);
        mixerBuffer.addFrom(0, 0, noiseBuffer, 0, 0, numSamples);
        
        // process the mixed buffer through a ladder filter
        ladderFilter.process(mixerContext);
        
		// copy the filtered buffer to the output buffer
        outputBuffer.addFrom(0, startSample, mixerBuffer, 0, 0, numSamples);
        
		// Se gli ADSR hanno finito la fase di decay (o se ho altri motivi per farlo)
		// segno la voce come libera per suonare altre note
        if (!ampAdsr.isActive() && noiseEnvFinished())
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
        
        // We prepare the release envelope generator of the noise osc
        noiseEnvelope.setSize(1, samplesPerBlock);    // mono --> modify? 
        noiseEnvelope.clear();
        egNoise.prepareToPlay(sampleRate);
        noiseFilter2.prepareToPlay(sampleRate);

		// Preparo le ProcessSpecs per l'oscillatore ed eventuali altre classi DSP
		spec.maximumBlockSize = samplesPerBlock;
		spec.sampleRate = sampleRate;
		spec.numChannels = 1;

		// Inizializzo l'oscillatore
        sawOscs.prepareToPlay(sampleRate, samplesPerBlock, spec);
        subOscillator.prepare(spec);
        ladderFilter.prepare(spec);
        noiseFilter.prepare(spec);
        noiseFilter.reset();

		// Se non ho intenzione di generare un segnale intrinsecamente
		// stereo è inutile calcolare più di un canale. Ne calcolo 1 e 
		// poi nel PluginProcessor lo copio su tutti i canali in uscita
        // WILL MODIFY THIS PART!
		oscillatorBuffer.setSize(1, samplesPerBlock);
        subBuffer.setSize(1, samplesPerBlock);
        noiseBuffer.setSize(1, samplesPerBlock);
        mixerBuffer.setSize(1, samplesPerBlock);
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
    
    void setNoiseRelease(const float newValue)
    {
        egNoise.setRelease(newValue);
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
        ladderFilter.setCutoffFrequencyHz(newValue);
    }
    
    void setQuality(const float newValue)
    {
        ladderFilter.setResonance(newValue);
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
        if (newValue < 0.45f)
        {           // low-pass
            noiseFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
            float noiseCutoff = juce::jmap(newValue, 0.0f, 0.45f, 80.0f, 18000.0f);
            noiseFilter.setCutoffFrequency(noiseCutoff);
        }else if(newValue < 0.55f)
        {           // all-pass
            noiseFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
            noiseFilter.setCutoffFrequency(20000.0f);
        }else
        {           // high-pass
            noiseFilter.setType(juce::dsp::StateVariableTPTFilterType::highpass);
            float cutoff = juce::jmap(newValue, 0.55f, 1.0f, 0.0f, 800.0f);
            noiseFilter.setCutoffFrequency(cutoff);
        }
        noiseFilter.reset();
    }
    
    bool noiseEnvFinished() const
    {
        return !egNoise.isActive();
    }
    
private:
	// La classe dsp::Oscillator può essere inizializzata con una lambda da usare come forma d'onda
	// (x va da -pi a + pi) e con un intero facoltativo che (se presente e diverso da 0) indica alla
	// classe di usare una lookup table di quelle dimensioni.
	// Se si vuole usare un oscillatore dalla forma d'onda variabile o con più parametri è meglio
	// implementarne uno come quello visto a lezione.
    
    dsp::ProcessSpec spec;
	

    SawOscillators sawOscs;
    
    // Sub Oscillator
    dsp::Oscillator<float> subOscillator{ [](float x) {return std::sin(x); } };

	// calculating cents to detune
	// 1 cent is 1/1200 octave
//	const double root = std::exp(std::log(2) / 1200);
//	// raise to the number of cents (default 15 sounds nice)
//	double cent = pow(root, 15);
    // to track detune, register parameters on active note
    int currentMidiNote = 60;

	// Linear ADSR --> modify: will add a param (slope) which will make the lines nice and exponential curves
	ADSR ampAdsr;
	ADSR::Parameters ampAdsrParams;
    
    bool trigger = false;
    // We use the JUCE class Random to generate noise
    Random noise;
    AudioBuffer<float> noiseEnvelope;
    // envelope generator for the noise osc
    ReleaseFilter egNoise;
    StereoFilter noiseFilter2;
    
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
    //float noiseFiltParam;
    
    // filters
    LadderFilter ladderFilter;                              // LPF for the whole synth
    juce::dsp::StateVariableTPTFilter<float> noiseFilter;   // LP/AP/HP filter for the noise oscillator
    
    // buffers
	AudioBuffer<float> oscillatorBuffer;
    AudioBuffer<float> subBuffer;
    AudioBuffer<float> noiseBuffer;
    AudioBuffer<float> mixerBuffer;
	float velocityLevel = 0.7f;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleSynthVoice)
};
