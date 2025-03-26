#pragma once
#include <JuceHeader.h>
#include "Filters.h"

// Sembrano classi lunghe, ma se eliminate i commenti diventa tutto molto snello
// ===================================================================================

// La classe Synthesiser da istanziare nel PluginProcessor si appoggia ad altre due
// classi astratte che dovrete implementare voi: SynthesiserSound e SynthesiserVoice.

// La prima serve semplicemente a mappare i canali midi e le zone della tastiera, mentre 
// la seconda implementa il vostro bellissimo algoritmo di sintesi.
// La gestione della polifonia è adibita alla classe Synthesiser, quello che implementate 
// in SynthesiserVoice è la SINGOLA VOCE DI POLIFONIA.
// Per l'implementazione di sint monofonici con meccanismi di note-priority occorre
// ereditare da Synthesiser e fare l'override di handleMidiEvent per implementare un meccanismo
// personalizzato di voice stealing (oppure non usate la classe Synthesiser).
//
// https://docs.juce.com/master/classSynthesiser.html

// ===================================================================================

// Un SynthesiserSound descrive un possibile suono riproducibile da un sintetizzatore.
// In questo esempio il sintetizzatore suona in modo polifonico un solo suono, ma
// se voleste spezzare la tastiera in due per fare in modo che la prima metà abbia
// un timbro e la seconda un altro (o se voleste che ogni canale MIDI fosse associato
// ad un suono diverso) dovreste implementare questa classe in modo diverso.
// Si pensi a questa classe come a uno strumento necessario per mappare un suono sulla tastiera
// o su uno o più canali MIDI. La classe che invece effettivamente suona è la SynthesiserVoice.
// In questo esempio un synth sound è associato a tutta la tastiera e a tutti i canali MIDI
//
// https://docs.juce.com/master/classSynthesiserSound.html
//

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

// ===================================================================================

// Una classe SynthesiserVoice rappresenta la singola voce di polifonia usata per generare un suono.
// Questa è la classe da implementare per creare il vostro sintetizzatore.
// In particolare la classe Synthesiser si occupa di gestire diverse istanze di SynthesiserVoice,
// ognuna adibita alla riproduzione di una voce di polifonia.
//
// https://docs.juce.com/master/classSynthesiserVoice.html
//

class SimpleSynthVoice : public SynthesiserVoice
{
public:
	SimpleSynthVoice(float defaultAtk = 0.005f, float defaultDcy = 0.025f, float defaultSus = 0.6f, float defaultRel = 0.7f, float defaultNoiseRel = 0.25f, float defaultSaw = 1.0f, float defaultSub = 0.0f, float defaultNoise = 0.0f)
    : ampAdsrParams(defaultAtk, defaultDcy, defaultSus, defaultRel), noiseAdsrParams(0.005f, 0.025f, 0.6f, defaultNoiseRel), sawGain(defaultSaw), subGain(defaultSub), noiseGain(defaultNoise)
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
        //noiseEnvelope.setSize(0, 0);
    }

	// Metodo chiamato dalla classe Synthesiser per ogni Note-on assegnato a questa voce
	void startNote(int midiNoteNumber, float velocity, SynthesiserSound* sound, int currentPitchWheelPosition) override
	{	
		// Reset phase for each oscillator
		for (auto& oscillator : oscillators)
		{
			oscillator.reset(); // got rid of pops
		}
        noiseOscillator.reset();
        subOscillator.reset();
        
		float baseFreq = MidiMessage::getMidiNoteInHertz(midiNoteNumber);
		// set the detuned oscillators' frequencies
		float detunedFreq1 = baseFreq * cent;
		float detunedFreq2 = baseFreq / cent;
        
        // modify: the sub's frequency for now, will just be one octave lower. I will add the register parameter later on
        float subFreq = baseFreq / 2;
        
        // Cambio frequenza all'oscillatore (il secondo argomento a true indica di NON usare smoothed value)
		// set the frequencies
		oscillators[0].setFrequency(baseFreq, true);
		oscillators[1].setFrequency(detunedFreq1, true);
		oscillators[2].setFrequency(detunedFreq2, true);
        
        subOscillator.setFrequency(subFreq);

		// Triggero l'ADSR
		ampAdsr.noteOn();
        noiseAdsr.noteOn();

		// Mi salvo la velocity da usare come volume della nota suonata
		velocityLevel = velocity;
	}
	
	// Metodo chiamato dalla classe Synthesiser per ogni Note-off assegnato a questa voce
	void stopNote(float velocity, bool allowTailOff) override
	{
		// Triggero la fase di release dell'ADSR
		ampAdsr.noteOff();
        noiseAdsr.noteOff();

		// Se mi chiede di non generare code (o altri casi, tipo se l'ADSR ha già finito la fase di release)
		// allora segnalo alla classe Synthesiser che questa voce è libera per poter riprodurre altri suoni
        if (!allowTailOff || ( !ampAdsr.isActive() && !noiseAdsr.isActive()))
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
        
        //DBG("Noise Gain: " << noiseGain);

		// Preparazione del ProcessContext per le classi DSP
		auto voiceData = oscillatorBuffer.getArrayOfWritePointers();
		dsp::AudioBlock<float> audioBlock{ voiceData, 1, (size_t)numSamples };
		dsp::ProcessContextReplacing<float> context{ audioBlock };
        
        auto subData = subBuffer.getArrayOfWritePointers();
        dsp::AudioBlock<float> subAudioBlock{ subData, 1, (size_t)numSamples };
        dsp::ProcessContextReplacing<float> subContext{ subAudioBlock };
        
        auto noiseData = noiseBuffer.getArrayOfWritePointers();
        dsp::AudioBlock<float> noiseAudioBlock{ noiseData, 1, (size_t)numSamples };
        dsp::ProcessContextReplacing<float> noiseContext{ noiseAudioBlock };

		// Genero la mie forme d'onda
        // SHOULD BECOME A NORMAL FOR LOOP WITH N (# OF VOICES) -- N will also become a param
		for (auto& oscillator : oscillators)
		{
			oscillator.process(context);
		}
        subOscillator.process(subContext);
        noiseOscillator.process(noiseContext);
        
        //noiseEnvelope.setSample(0, timeStamp, velocity);
        //egNoise.processBlock(noiseEnvelope, numSamples);

		// [Solitamente qui ci stanno cose tipo mixer degli oscillatori, filtro e saturazione]

        
		// La modulo in ampiezza con l'ADSR
		ampAdsr.applyEnvelopeToBuffer(oscillatorBuffer, 0, numSamples);
        ampAdsr.applyEnvelopeToBuffer(subBuffer, 0, numSamples);
        noiseAdsr.applyEnvelopeToBuffer(noiseBuffer, 0, numSamples);
        //noiseAdsr.applyEnvelopeToBuffer(oscillatorBuffer, 0, numSamples);

		// Volume proporzionale alla velocity
		oscillatorBuffer.applyGain(0, numSamples, velocityLevel * sawGain);
        subBuffer.applyGain(0, numSamples, velocityLevel * subGain);
        noiseBuffer.applyGain(0, numSamples, velocityLevel * noiseGain);

		// Copio il segnale generato nel buffer di output considerando la porzione di competenza
		outputBuffer.addFrom(0, startSample, oscillatorBuffer, 0, 0, numSamples);
        outputBuffer.addFrom(0, startSample, subBuffer, 0, 0, numSamples);
        outputBuffer.addFrom(0, startSample, noiseBuffer, 0, 0, numSamples);

		// Se gli ADSR hanno finito la fase di decay (o se ho altri motivi per farlo)
		// segno la voce come libera per suonare altre note
        if (!ampAdsr.isActive() && !noiseAdsr.isActive())
			clearCurrentNote();
	}

	// ==== Metodi personali ====

	// Prepare to play della singola voce di polifonia. Non è prevista dalla classe astratta
	// (c'è un metodo setSampleFrequency o qualcosa del genere che svolge una funzione simile)
	// ma è comodo averla per poter usare il sint come un normale Processor
	void prepareToPlay(double sampleRate, int samplesPerBlock)
	{
		// Resetto gli ADSR
		ampAdsr.setSampleRate(sampleRate);
		ampAdsr.setParameters(ampAdsrParams);
        
        noiseAdsr.setSampleRate(sampleRate);
        noiseAdsr.setParameters(noiseAdsrParams);
        
        
        // We prepare the release envelope generator of the noise osc
//        egNoise.prepareToPlay(sampleRate);
//        noiseEnvelope.setSize(1, samplesPerBlock);    // mono
//        noiseEnvelope.clear();

		// Preparo le ProcessSpecs per l'oscillatore ed eventuali altre classi DSP
		dsp::ProcessSpec spec;
		spec.maximumBlockSize = samplesPerBlock;
		spec.sampleRate = sampleRate;
		spec.numChannels = 1;

		// Inizializzo l'oscillatore
		for (auto& oscillator : oscillators)
		{
			oscillator.prepare(spec);
		}
        subOscillator.prepare(spec);
        noiseOscillator.prepare(spec);

		// Se non ho intenzione di generare un segnale intrinsecamente
		// stereo è inutile calcolare più di un canale. Ne calcolo 1 e 
		// poi nel PluginProcessor lo copio su tutti i canali in uscita
        // WILL MODIFY THIS PART!
		oscillatorBuffer.setSize(1, samplesPerBlock);
        subBuffer.setSize(1, samplesPerBlock);
        noiseBuffer.setSize(1, samplesPerBlock);
	}
	
    // Parameter setters
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
        //egNoise.setRelease(newValue);
        noiseAdsrParams.release = newValue;
        noiseAdsr.setParameters(noiseAdsrParams);
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

private:
	// La classe dsp::Oscillator può essere inizializzata con una lambda da usare come forma d'onda
	// (x va da -pi a + pi) e con un intero facoltativo che (se presente e diverso da 0) indica alla
	// classe di usare una lookup table di quelle dimensioni.
	// Se si vuole usare un oscillatore dalla forma d'onda variabile o con più parametri è meglio
	// implementarne uno come quello visto a lezione.
	
	// sine wave
	//dsp::Oscillator<float> sinOscillator{ [](float x) {return std::sin(x); } };

	dsp::Oscillator<float> oscillators[3] =
	{
		{ [](float x) {return x / MathConstants<float>::pi; }},	// main saw osc
		{ [](float x) {return x / MathConstants<float>::pi; }}, // 1st detuned
		{ [](float x) {return x / MathConstants<float>::pi; }}
        // to obtain the JP8000 supersaw 4 more detuned oscillators should be added
//        , { [](float x) {return std::sin(x); }}// sub sine
	};
    
    // Sub Oscillator
    dsp::Oscillator<float> subOscillator{ [](float x) {return std::sin(x); } };
    
    // Noise Oscillator
    dsp::Oscillator<float> noiseOscillator{[this](float) {return noise.nextFloat() * 2.0f - 1.0f;}};

	// calculating cents to detune
	// 1 cent is 1/1200 octave
	const double root = std::exp(std::log(2) / 1200);
	// amt of detune is fixed (for now) for the 2nd & 3rd oscillators
	// raise to the number of cents (15 sounds nice)
	double cent = pow(root, 15);

	// La classe di JUCE ADSR mette a disposizione un generatore di inviluppo a 4 
	// sezioni (Attack, Decay, Sustain, Release) e una struttura dati che permette
	// di cambiare i 4 parametri.
	// Onestamente non mi piace molto l'ADSR di JUCE per la modulazione
	// dell'inviluppo d'ampiezza, perchè le fasi di A, D e R sono di tipo lineare,
	// mentre suonerebbe più naturale averle di tipo esponenziale...
	ADSR ampAdsr;
	ADSR::Parameters ampAdsrParams;
    // ADSR con solo il parametro Release per il noise osc
    ADSR noiseAdsr;
    ADSR::Parameters noiseAdsrParams;
    
    // We use the JUCE class Random to generate noise
    Random noise;
    //AudioBuffer<float> noiseEnvelope;
    
    // envelope generator for the noise osc
    //ReleaseFilter egNoise;
    
    // osc level params --> MODIFY: might make a mixer class
    float sawGain;
    float subGain;
    float noiseGain;
	
	AudioBuffer<float> oscillatorBuffer;
    AudioBuffer<float> subBuffer;
    AudioBuffer<float> noiseBuffer;
	float velocityLevel = 0.7f;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleSynthVoice)
};
