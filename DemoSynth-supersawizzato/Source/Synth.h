#pragma once
#include <JuceHeader.h>
#include "Oscillators.h"
#include "Filters.h"
#include "MyADSR.h"

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
	SimpleSynthVoice( int defaultSawNum = 5, int defaultDetune = 15, float defaultPhase = 0.0f, float defaultStereoWidth = 0.0f, float defaultAtk = 0.005f, float defaultDcy = 0.025f, float defaultSus = 0.6f, float defaultRel = 0.7f, float defaultSaw = 1.0f, float defaultSub = 0.0f, float defaultNoise = 0.0f,  int defaultSubReg = 0, float defaultEnvAmt = 0.0f, double defaultLfoFreq = 20.0, int defaultLfoWf = 0 /*, int defaultSubWf = 0*/)
    : sawOscs(defaultSawNum, defaultDetune, defaultPhase, defaultStereoWidth), ampAdsrParams(defaultAtk, defaultDcy, defaultSus, defaultRel),  sawGain(defaultSaw), subGain(defaultSub), noiseGain(defaultNoise), subRegister(defaultSubReg), egAmt(defaultEnvAmt), lfo(defaultLfoFreq, defaultLfoWf)/*, subWaveform(defaultSubWf)*/
	{
	};
	
	~SimpleSynthVoice() {};

	// ==== Metodi astratti ereditati da SynthesiserVoice che devono essere implementati ====

	// Deve ritornare true se questa voce è adibita alla riproduzione di un certo SynthSound.
	bool canPlaySound(SynthesiserSound* sound) override
	{
		return dynamic_cast<MySynthSound*>(sound) != nullptr;
	}
    
    void releaseResources()
    {
        noiseOsc.releaseResources();
        oscillatorBuffer.setSize(0, 0);
        subBuffer.setSize(0, 0);
        noiseBuffer.setSize(0, 0);
        mixerBuffer.setSize(0, 0);
        modulation.setSize(0, 0);
    }

	// Metodo chiamato dalla classe Synthesiser per ogni Note-on assegnato a questa voce
	void startNote(int midiNoteNumber, float velocity, SynthesiserSound* sound, int currentPitchWheelPosition) override
	{
		// Reset phase for each oscillator
        subOscillator.reset();
        sawOscs.startNote(midiNoteNumber);
        
        // storing currentMidiNote for parameter changes related to the frequency
        currentMidiNote = midiNoteNumber;
        updateFreqs();  // this is mainly used so the sub can calculate its freq

		// Trigger the ADSR
		ampAdsr.noteOn();
        
		// Mi salvo la velocity da usare come volume della nota suonata
		velocityLevel = velocity;
        trigger = true;
	}
	
	// Metodo chiamato dalla classe Synthesiser per ogni Note-off assegnato a questa voce
	void stopNote(float velocity, bool allowTailOff) override
	{
		// Trigger the release phase of the ADSR
		ampAdsr.noteOff();

		// signaling that this voice is now free process new sounds
        if (!allowTailOff || ( !ampAdsr.isActive() /*&& noiseOsc.envFinished()*/))
			clearCurrentNote();
	}
	
	// Metodo chiamato dalla classe Synthesiser per ogni Control Change ricevuto
	void controllerMoved(int controllerNumber, int newControllerValue) override {}
	
	// Metodo chiamato dalla classe Synthesiser in caso venga mossa la pitch-wheel
	void pitchWheelMoved(int newPitchWheelValue) override {}

	void renderNextBlock(AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
	{
        // calculates the audio block and returns the last sample
        float lfoVal = lfo.getNextAudioBlock(modulation, numSamples);
        
		// Se la voce non è attiva ci si può fermare qui
		if (!isVoiceActive())
			return;

		// Clearing buffers to be processed
		oscillatorBuffer.clear(0, numSamples);
        subBuffer.clear(0, numSamples);
        noiseBuffer.clear(0, numSamples);
        mixerBuffer.clear(0, numSamples);

		// Preparazione del ProcessContext per le classi DSP
		auto voiceData = oscillatorBuffer.getArrayOfWritePointers();
        
		dsp::AudioBlock<float> audioBlock{ voiceData, 2, (size_t)numSamples };
		dsp::ProcessContextReplacing<float> context{ audioBlock };
        
        auto subData = subBuffer.getArrayOfWritePointers();
        dsp::AudioBlock<float> subAudioBlock{ subData, 1, (size_t)numSamples };
        dsp::ProcessContextReplacing<float> subContext{ subAudioBlock };

        auto mixerData = mixerBuffer.getArrayOfWritePointers();
        dsp::AudioBlock<float> mixerBlock{ mixerData, 2, (size_t)numSamples };
        dsp::ProcessContextReplacing<float> mixerContext{ mixerBlock };

        // recalculate detune frequencies etc. if in case they have been changed after the note was triggered
        sawOscs.updateFreqs();
        
        // 2X OVERSAMPLING -- generate sounds
        // oversampling begins
        auto oversampledBlock = oversampler->processSamplesUp (audioBlock);
        // process oversampled block with twice the number of samples
        sawOscs.processStereo(oversampledBlock, numSamples * 2);
        // decimate the samples back to the original sample rate
        oversampler->processSamplesDown (audioBlock);
                
        subOscillator.process(subContext);
        
        // noise: trigger the ReleaseFilter envelope
        if(trigger)
        {
            noiseOsc.trigger(startSample, velocityLevel);
            trigger = false;
        }
        
        // process the noise
        noiseOsc.process(noiseBuffer, startSample, numSamples, noiseGain);
        // filtering the noise with its parameter before the main LPF
        noiseFilter.processBlock(noiseBuffer, numSamples);

		// Volume proporzionale alla velocity
        for (int ch = 0; ch < oscillatorBuffer.getNumChannels(); ++ch)
        {
            oscillatorBuffer.applyGain(ch, 0, numSamples, velocityLevel * sawGain / std::sqrt(sawOscs.getActiveOscs()));
        }
        subBuffer.applyGain(0, numSamples, velocityLevel * subGain);
        
        // mix all buffers into one
        mixerBuffer.addFrom(0, 0, oscillatorBuffer, 0, 0, numSamples);
        mixerBuffer.addFrom(1, 0, oscillatorBuffer, 1, 0, numSamples);

        // Then: add mono sub and noise to *both* channels
        for (int ch = 0; ch < 2; ++ch)
        {
            mixerBuffer.addFrom(ch, 0, subBuffer, 0, 0, numSamples);
            mixerBuffer.addFrom(ch, 0, noiseBuffer, 0, 0, numSamples);
        }
        
        // FILTERING - process the mixed buffer through a ladder filter
//        ladderFilter.process(mixerContext);
        // to filter with the EG and LFO, we must get ADSR and LFO values then modulate the cutoff with their values
        ladderFilter.processWithEG(mixerContext, ampAdsr, lfoVal, numSamples);

        // La modulo in ampiezza con l'ADSR
        ampAdsr.applyEnvelopeToBuffer(mixerBuffer, 0, numSamples);
        
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
        
        lfo.prepareToPlay(sampleRate);

		// Preparo le ProcessSpecs per l'oscillatore ed eventuali altre classi DSP
        // for the mono sounds, i.e. sub and noise
        spec.maximumBlockSize = samplesPerBlock;
		spec.sampleRate = sampleRate;
		spec.numChannels = 1;
        // for the oversampled saw waves
        stereoOversampledSpec.maximumBlockSize = samplesPerBlock * 2;
        stereoOversampledSpec.sampleRate = sampleRate * 2;
        stereoOversampledSpec.numChannels = 2;
        // for the ladder filter
        specStereo.maximumBlockSize = samplesPerBlock;
        specStereo.sampleRate = sampleRate;
        specStereo.numChannels = 2;
        
        // set up the oversampler with the same specs -- the 1 is the oversampling factor where 2^n
        oversampler = std::make_unique<dsp::Oversampling<float>>(stereoOversampledSpec.numChannels, 1, dsp::Oversampling<float>::filterHalfBandPolyphaseIIR);
        oversampler->initProcessing(samplesPerBlock);
        oversampler->reset();
        
		// initializing oscillators, noise generator and filters
        sawOscs.prepareToPlay(stereoOversampledSpec);
        subOscillator.prepare(spec);

        ladderFilter.prepare(specStereo);

        noiseOsc.prepareToPlay(spec);
        noiseFilter.prepareToPlay(spec);

		oscillatorBuffer.setSize(2, samplesPerBlock);
        subBuffer.setSize(1, samplesPerBlock);
        noiseBuffer.setSize(1, samplesPerBlock);
        mixerBuffer.setSize(2, samplesPerBlock);
        modulation.setSize(2, samplesPerBlock);
	}
    
    
    void updatePosition(AudioPlayHead::CurrentPositionInfo newPosition)
    {
        lfo.updatePosition(newPosition);
    }
    
    void updateFreqs()
    {
//        if (isVoiceActive()) {
            float baseFreq = MidiMessage::getMidiNoteInHertz(currentMidiNote) * std::pow(2, sawRegister + 1);
            sawOscs.setSawFreqs(baseFreq);
            
            float subFreq = baseFreq / std::pow(2, subRegister + 3);
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
        sawGain = newValue;
    }
    
    void setSubGain(const float newValue)
    {
        subGain = newValue;
    }
    
    void setNoiseGain(const float newValue)
    {
        noiseGain = newValue;
    }
    
	// Setters of ADSR parameters
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
		ampAdsrParams.sustain = sqrt(newValue);
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
                case 0: // sinusoidal
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
        subOscillator.prepare(spec);
        updateFreqs();
    }
    
    void setCutoff(const float newValue)
    {
        ladderFilter.setCutoff(newValue);
    }
    
    void setQuality(const float newValue)
    {
        ladderFilter.setResonance(newValue);
    }
    
    void setFilterEnvAmt(const float newValue)
    {
        ladderFilter.setEnvAmt(newValue);
//        egAmt = newValue;
    }
    
    void setFilterLfoAmt(const float newValue)
    {
        ladderFilter.setLfoAmt(newValue);
    }
    
    void setLfoWf(const int newValue)
    {
        lfo.setWaveform(newValue);
    }
    
    void setLfoFreq(const float newValue)
    {
        lfo.setFrequency(newValue);
    }
    
    void setLfoRate(const float newValue)
    {
        lfo.setRate(newValue);
    }
    
    void setLfoSync(const bool newValue)
    {
        lfo.setSyncOn(newValue);
    }
    
    void setNoiseFilterCutoff(const float newValue)
    {
        noiseFilter.setFrequency(newValue);
    }
    
    void setNoiseRelease(const float newValue)
    {
        noiseOsc.setRelease(newValue);
    }
    
private:
    dsp::ProcessSpec spec;
    dsp::ProcessSpec specStereo;
    dsp::ProcessSpec stereoOversampledSpec;
    
    std::unique_ptr<dsp::Oversampling<float>> oversampler;

    SawOscillators sawOscs;
    NoiseOsc noiseOsc;
    // Sub Oscillator
    dsp::Oscillator<float> subOscillator{ [](float x) {return std::sin(x); } };
    NaiveOscillator lfo;

    // to track detune, register parameters on active note
    int currentMidiNote = 60;

	// Linear ADSR --> modify: will add a param (slope) which will make the lines nice and exponential curves
	MyADSR ampAdsr;
	ADSR::Parameters ampAdsrParams;
    
    // used for triggering the noise envelope
    bool trigger = false;
    
    // osc params
    int sawRegister = 0;
    
    // osc level params
    float sawGain;
    float subGain;
    float noiseGain;
    
    // sub params
    int subRegister;
    
    // modify: are these necessary? for initial parameter values in the interface... maybe? -- have to check
    //int subWaveform;
    
    // filters
    LadderFilter ladderFilter;          // LPF for the whole synth
    StereoFilter noiseFilter;           // filter for noise
    float egAmt = 0.0f;
    
    // buffers
	AudioBuffer<float> oscillatorBuffer;
    AudioBuffer<float> subBuffer;
    AudioBuffer<float> noiseBuffer;
    AudioBuffer<float> mixerBuffer;
    AudioBuffer<double> modulation;
	float velocityLevel = 0.7f;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleSynthVoice)
};
