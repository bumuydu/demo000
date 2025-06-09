#pragma once
#include <JuceHeader.h>
#include "Oscillators.h"
#include "Filters.h"
#include "MyADSR.h"
#include "Mixer.h"
#include "Oversampling.h"

#define VELOCITY_DYN_RANGE 9.0f  //dB;

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
	SimpleSynthVoice( int defaultSawNum = 5, int defaultDetune = 15, float defaultPhase = 0.0f, float defaultStereoWidth = 0.0f,
                     int defaultSubReg = 2, float defaultEnvAmt = 0.0f, double defaultLfoFreq = 0.01, int defaultLfoWf = 0)
    : sawOscs(defaultSawNum, defaultDetune, defaultStereoWidth), subRegister(defaultSubReg), egAmt(defaultEnvAmt), subOscillator(20.0, 0), lfo(defaultLfoFreq, defaultLfoWf)
	{
	};
	
	~SimpleSynthVoice() {};

	bool canPlaySound(SynthesiserSound* sound) override
	{
		return dynamic_cast<MySynthSound*>(sound) != nullptr;
	}
    
    void releaseResources()
    {
        sawOscs.releaseResources();
        noiseOsc.releaseResources();
        oversmpBuffer.setSize(0, 0);
        oscillatorBuffer.setSize(0, 0);
        subBuffer.setSize(0, 0);
        noiseBuffer.setSize(0, 0);
        mixerBuffer.setSize(0, 0);
        modulation.setSize(0, 0);
        frequencyBuffer.setSize(0, 0);
    }

	void startNote(int midiNoteNumber, float velocity, SynthesiserSound* sound, int currentPitchWheelPosition) override
	{
//        oversmpBuffer.clear();
//        oscillatorBuffer.clear();
//        subBuffer.clear();
//        noiseBuffer.clear();
//        mixerBuffer.clear();
        
		// Reset phase for each oscillator
        subOscillator.resetPhase();
        sawOscs.startNote(nn2hz(currentMidiNote) * std::pow(2, sawRegister + 1));
        oSmp.resetFilter(); // modify: delete if not needed
        
        // storing currentMidiNote for parameter changes related to the frequency
        noteNumber.setTargetValue(midiNoteNumber);
        currentMidiNote = midiNoteNumber;
        updateFreqs();  // this is mainly used so the sub can calculate its freq

		// Trigger the ADSR
		ampAdsr.noteOn();
        velocityLevel = Decibels::decibelsToGain(velocity * VELOCITY_DYN_RANGE - VELOCITY_DYN_RANGE);
//		velocityLevel = velocity;
//        mixer.updateGain();
        trigger = true;
	}
	
	void stopNote(float velocity, bool allowTailOff) override
	{
		// Trigger the release phase of the ADSR
		ampAdsr.noteOff();

		// signaling that this voice is now free process new sounds
        if (!allowTailOff || ( !ampAdsr.isActive() /*&& noiseOsc.envFinished()*/))
			clearCurrentNote();
	}
	
	void controllerMoved(int controllerNumber, int newControllerValue) override {}
	
	void pitchWheelMoved(int newPitchWheelValue) override {}

	void renderNextBlock(AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
	{
        lfo.getNextAudioBlock(modulation, startSample, numSamples);
        
        // modify: I might move this to after the initial return
        frequencyModulation(startSample, numSamples);
        
		if (!isVoiceActive())
			return;
        
        oversmpBuffer.clear();
		oscillatorBuffer.clear();
        subBuffer.clear();
        noiseBuffer.clear();
        mixerBuffer.clear();

        const int startSampleOS = startSample * oversamplingFactor;
        const int numSamplesOS = numSamples * oversamplingFactor;
        
        // 2X OVERSAMPLING -- generate sounds at oversampled sample rate and decimate to original sample rate
        sawOscs.process(oversmpBuffer, frequencyBuffer, startSampleOS, numSamplesOS);
        oSmp.filterAndDecimate(oversmpBuffer, oscillatorBuffer, startSampleOS, numSamplesOS, oversamplingFactor);
        
        subOscillator.getNextAudioBlockFloat(subBuffer, startSample, numSamples);
        // noise: trigger the ReleaseFilter envelope
        if(trigger)
        {
            noiseOsc.trigger(startSample, velocityLevel);
            trigger = false;
        }
        noiseOsc.process(noiseBuffer, startSample, numSamples, mixer.getNoiseGain());
        // filtering the noise with its parameter before the main LPF
//        noiseFilter.processBlock(noiseBuffer, numSamples);
        noiseFilter.processBlock(noiseBuffer, startSample, numSamples);

        mixer.getNextAudioBlock(mixerBuffer, oscillatorBuffer, subBuffer, noiseBuffer, startSample, numSamples, velocityLevel, sawOscs.getActiveOscs());
        
        // FILTERING - process the mixed buffer through a ladder filter
        // to filter with the EG and LFO, we must get ADSR and LFO values then modulate the cutoff with their values
        moogFilter.process(mixerBuffer, ampAdsr, modulation, startSample, numSamples);

        ampAdsr.applyEnvelopeToBuffer(mixerBuffer, startSample, numSamples);
         
        mixer.applyMasterGainAndCopy(outputBuffer, mixerBuffer, startSample, numSamples);

		// Se gli ADSR hanno finito la fase di decay (o se ho altri motivi per farlo)
		// segno la voce come libera per suonare altre note
        if (!ampAdsr.isActive() && noiseOsc.envFinished())
        {
            clearCurrentNote();
            trigger = false;
        }
	}

	// ==== Metodi personali ====

	void prepareToPlay(double sampleRate, int samplesPerBlock)
	{
        const int sampleRateOs = sampleRate * oversamplingFactor;
        const int samplesPerBlockOs = samplesPerBlock * oversamplingFactor;
        // for the mono sounds, i.e. sub and noise
        spec.maximumBlockSize = samplesPerBlock;
		spec.sampleRate = sampleRate;
		spec.numChannels = 1;
        // for the oversampled saw waves
        stereoOversampledSpec.maximumBlockSize = samplesPerBlockOs;
        stereoOversampledSpec.sampleRate = sampleRateOs;
        stereoOversampledSpec.numChannels = 2;
             
        oversmpBuffer.setSize(2, samplesPerBlockOs);
        oscillatorBuffer.setSize(2, samplesPerBlock);
        subBuffer.setSize(1, samplesPerBlock);
        noiseBuffer.setSize(1, samplesPerBlock);
        mixerBuffer.setSize(2, samplesPerBlock);
        modulation.setSize(2, samplesPerBlock);
        frequencyBuffer.setSize(1, samplesPerBlockOs);
        
        // initializing oscillators, noise generator and filters, mixer etc.
        oSmp.prepareToPlay(sampleRateOs, sampleRate);
        sawOscs.prepareToPlay(stereoOversampledSpec);
        subOscillator.prepareToPlay(sampleRate);
        noiseOsc.prepareToPlay(spec);
        noiseFilter.prepareToPlay(spec);
        moogFilter.prepareToPlay(sampleRate);
        lfo.prepareToPlay(sampleRate);
        ampAdsr.prepareToPlay(sampleRate);
        mixer.prepareToPlay(sampleRate);
        noteNumber.reset(sampleRate, 0.001f);        
	}
    
    void updatePosition(AudioPlayHead::CurrentPositionInfo newPosition)
    {
        lfo.updatePosition(newPosition);
    }
	
    // Parameter setters
    
    void setMainWf(const int newValue)
    {
        sawOscs.setWf(newValue);
    }
    
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
    
    void setSawPhase(const int newValue)
    {
        sawOscs.setPhaseDegree(newValue);
    }
    
    void setPhaseResetting(const bool newValue)
    {
        sawOscs.setPhaseResetting(newValue);
    }
    
    void setSawGain(const float newValue)
    {
        mixer.setSawGain(newValue);
    }
     
    void setSubGain(const float newValue)
    {
        mixer.setSubGain(newValue);
    }
    
    void setNoiseGain(const float newValue)
    {
        mixer.setNoiseGain(newValue);
    }
    
	// Setters of ADSR parameters
	void setAttack(const float newValue)
	{
        ampAdsr.setAttack(newValue);
	}

	void setDecay(const float newValue)
	{
        ampAdsr.setDecay(newValue);
	}

	void setSustain(const float newValue)
	{
        ampAdsr.setSustain(newValue);
	}

	void setRelease(const float newValue)
	{
        ampAdsr.setRelease(newValue);
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
            subOscillator.setWaveform(0);
            break;
        case 1: // square
            subOscillator.setWaveform(3);
            break;
        default:
            subOscillator.setWaveform(0);
            break;
        }
        updateFreqs();
    }
    
    void setCutoff(const float newValue)
    {
        moogFilter.setCutoff(newValue);
    }
    
    void setQuality(const float newValue)
    {
        moogFilter.setResonance(newValue);
    }
    
    void setFilterEnvAmt(const float newValue)
    {
        moogFilter.setEnvAmt(newValue);
//        egAmt = newValue;
    }
    
    void setFilterLfoAmt(const float newValue)
    {
        moogFilter.setLfoAmt(newValue);
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
    
    void setOversampling(const int newValue)
    {
        // parameter is choice = values are index values 0 and 1
//        oversamplingFactor = (newValue + 1) * 2;
        // modify: interrupt sounds and processing? -- then, reset the prepareToPlay?
//        prepareToPlay(spec.sampleRate, spec.maximumBlockSize);
    }
    
    void setMasterGain(const float newValue)
    {
        mixer.setMasterGain(newValue);
    }
    
private:
    
    void updateFreqs()
    {
        double baseFreq = nn2hz(currentMidiNote) * std::pow(2, sawRegister + 1);
        double subFreq = baseFreq / std::pow(2, 5 - subRegister);
        subOscillator.setFrequency(subFreq);
    }
    
    double nn2hz(double nn)
    {
        return pow(2.0, (nn - 69.0) / 12.0) * 440.0;
    }
    
    void frequencyModulation(int startSample, int numSamples)
    {
        auto fmOsc1Data = frequencyBuffer.getArrayOfWritePointers();

        for (int i = startSample; i < numSamples; ++i)
        {
            const double currentNoteNumber = noteNumber.getNextValue();
            const double note = nn2hz(currentNoteNumber) * std::pow(2, sawRegister - 1);
            for (int j = 0; j < oversamplingFactor; ++j)
            {
                fmOsc1Data[0][(i * oversamplingFactor) + j] = note;
            }
        }
    }
    
    dsp::ProcessSpec spec;
    dsp::ProcessSpec stereoOversampledSpec;
    Oversampling oSmp;
    int oversamplingFactor = 2;
    
    SawOscillators sawOscs;
    NoiseOsc noiseOsc;
    NaiveOscillator subOscillator;
    NaiveOscillator lfo;
    
    // to track detune, register parameters on active note
    int sawRegister = 0;
    int subRegister;
    int currentMidiNote = 60;
    SmoothedValue<double, ValueSmoothingTypes::Linear> noteNumber;
    AudioBuffer<double> frequencyBuffer;

	MyADSR ampAdsr;         // double ADSR
    bool trigger = false;   // used for triggering the noise envelope
    Mixer mixer;
    
    // filters
    MoogFilters moogFilter;             // LPF for the whole synth
    NoiseFilter noiseFilter;           // filter for noise
    float egAmt = 0.0f;
    
    // buffers
	AudioBuffer<float> oscillatorBuffer;
    AudioBuffer<float> oversmpBuffer;
    AudioBuffer<float> subBuffer;
    AudioBuffer<float> noiseBuffer;
    AudioBuffer<float> mixerBuffer;
    AudioBuffer<double> modulation;
	float velocityLevel = 0.7f;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleSynthVoice)
};
