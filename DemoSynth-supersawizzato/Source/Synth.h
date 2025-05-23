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
	SimpleSynthVoice( int defaultSawNum = 5, int defaultDetune = 15, float defaultPhase = 0.0f, float defaultStereoWidth = 0.0f, float defaultSaw = 1.0f, float defaultSub = 0.0f, float defaultNoise = 0.0f,  int defaultSubReg = 0, float defaultEnvAmt = 0.0f, double defaultLfoFreq = 0.01, int defaultLfoWf = 0 /*, int defaultSubWf = 0*/)
    : /*sawOscs(defaultSawNum, defaultDetune, defaultPhase, defaultStereoWidth)*/sawOscs(defaultSawNum, defaultDetune, defaultStereoWidth),  sawGain(defaultSaw), subGain(defaultSub), noiseGain(defaultNoise), subRegister(defaultSubReg), egAmt(defaultEnvAmt), subOscillator(20.0, 0), lfo(defaultLfoFreq, defaultLfoWf)/*, subWaveform(defaultSubWf)*/
	{
//        sawGainn.setCurrentAndTargetValue(Decibels::decibelsToGain(-2.0f));
        masterGain.setCurrentAndTargetValue(Decibels::decibelsToGain(0.0f));
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
		// Reset phase for each oscillator
        subOscillator.resetPhase();
        sawOscs.startNote(nn2hz(currentMidiNote) * std::pow(2, sawRegister + 1));
        
        // storing currentMidiNote for parameter changes related to the frequency
        noteNumber.setTargetValue(midiNoteNumber);
        currentMidiNote = midiNoteNumber;
        updateFreqs();  // this is mainly used so the sub can calculate its freq

		// Trigger the ADSR
		ampAdsr.noteOn();
		velocityLevel = velocity;
//        updateGain();
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
        // calculates the audio block and returns the last sample
//        float lfoVal = lfo.getNextAudioBlock(modulation, numSamples);
        float lfoVal = lfo.getNextAudioBlock(modulation, startSample, numSamples);
        
        frequencyModulation(startSample, numSamples);
        
		if (!isVoiceActive())
			return;
        
        oversmpBuffer.clear();
		oscillatorBuffer.clear();
        subBuffer.clear();
        noiseBuffer.clear();
        mixerBuffer.clear();
        
        // 2X OVERSAMPLING -- generate sounds at oversampled sample rate and decimate to original sample rate
        // process oversampled block with twice the number of samples
        
//        sawOscs.process(oscillatorBuffer, frequencyBuffer, startSample, numSamples);
        sawOscs.process(oversmpBuffer, frequencyBuffer, (startSample * oversamplingFactor), (numSamples * oversamplingFactor));
        filterAndDecimate(oversmpBuffer, oscillatorBuffer, (startSample * oversamplingFactor), numSamples * oversamplingFactor);
        
        subOscillator.getNextAudioBlockFloat(subBuffer, startSample, numSamples);
        // noise: trigger the ReleaseFilter envelope
        if(trigger)
        {
            noiseOsc.trigger(startSample, velocityLevel);
            trigger = false;
        }
        noiseOsc.process(noiseBuffer, startSample, numSamples, noiseGain);
        // filtering the noise with its parameter before the main LPF
//        noiseFilter.processBlock(noiseBuffer, numSamples);
        noiseFilter.processBlock(noiseBuffer, startSample, numSamples);

		// Volume proporzionale alla velocity
        for (int ch = 0; ch < 2; ++ch)
        {
            oscillatorBuffer.applyGain(ch, startSample, numSamples, velocityLevel * sawGain / std::sqrt(sawOscs.getActiveOscs()));
//            sawGainn.applyGain(oscillatorBuffer.getWritePointer(ch) + startSample, numSamples);
        }
        
        subBuffer.applyGain(startSample, numSamples, velocityLevel * subGain);
        
        // mix all buffers into one
//        mixerBuffer.addFrom(0, startSample, oscillatorBuffer, 0, startSample, numSamples);
//        mixerBuffer.addFrom(1, startSample, oscillatorBuffer, 1, startSample, numSamples);
        for (int ch = 0; ch < 2; ++ch)
        {
            mixerBuffer.addFrom(ch, startSample, oscillatorBuffer, ch, startSample, numSamples);
            mixerBuffer.addFrom(ch, startSample, subBuffer, 0, startSample, numSamples);
            mixerBuffer.addFrom(ch, startSample, noiseBuffer, 0, startSample, numSamples);
        }
        
        // FILTERING - process the mixed buffer through a ladder filter
        // to filter with the EG and LFO, we must get ADSR and LFO values then modulate the cutoff with their values
        moogFilter.process(mixerBuffer, ampAdsr, lfoVal, startSample, numSamples);

        // La modulo in ampiezza con l'ADSR
        ampAdsr.applyEnvelopeToBuffer(mixerBuffer, startSample, numSamples);
                
        for (int ch = 0; ch < 2; ++ch)
        {
            masterGain.applyGain(mixerBuffer.getWritePointer(ch) + startSample, numSamples);
        }

		// copy the filtered buffer to the output buffer
        for (int ch = 0; ch < 2; ++ch)
        {
            outputBuffer.addFrom(ch, startSample, mixerBuffer, ch, startSample, numSamples);
        }
		// Se gli ADSR hanno finito la fase di decay (o se ho altri motivi per farlo)
		// segno la voce come libera per suonare altre note
        if (!ampAdsr.isActive() && noiseOsc.envFinished())
        {
            clearCurrentNote();
            trigger = false;
        }
	}
    
    void filterAndDecimate(AudioBuffer<float>& oversmpBuf, AudioBuffer<float>& output, int startSampleOs, int numSamplesOs)
    {
        auto* left = oversmpBuf.getWritePointer(0);
        auto* right = oversmpBuf.getWritePointer(1);
        auto* inL = oversmpBuf.getReadPointer(0);
        auto* outL = output.getWritePointer(0);
        auto* inR = oversmpBuf.getReadPointer(1);
        auto* outR = output.getWritePointer(1);

        const int endSampleOs = startSampleOs + numSamplesOs;
        // modify: if coefficients change dynamically --> reset
//        iirFilters[ch].reset();
        
        // filter all samples of the oversampled buffer
        for (int smp = startSampleOs; smp < endSampleOs; ++smp)
        {
            left[smp] = antialiasingFilterL.processSample(inL[smp]);
            right[smp] = antialiasingFilterR.processSample(inR[smp]);
        }

        // decimate
        const int startSampleOriginal = startSampleOs / oversamplingFactor;
        const int numSamplesOriginal = numSamplesOs / oversamplingFactor;
        const int endSampleOriginal = startSampleOriginal + numSamplesOriginal;
        for (int i = startSampleOriginal; i < endSampleOriginal; ++i)
        {
            outL[i] = inL[i * oversamplingFactor];
            outR[i] = inR[i * oversamplingFactor];
        }
    }

	// ==== Metodi personali ====

	void prepareToPlay(double sampleRate, int samplesPerBlock)
	{
        // for the mono sounds, i.e. sub and noise
        spec.maximumBlockSize = samplesPerBlock;
		spec.sampleRate = sampleRate;
		spec.numChannels = 1;
        // for the oversampled saw waves
        stereoOversampledSpec.maximumBlockSize = samplesPerBlock * oversamplingFactor;
        stereoOversampledSpec.sampleRate = sampleRate * oversamplingFactor;
        stereoOversampledSpec.numChannels = 2;
        // for the stereo LPF
        specStereo.maximumBlockSize = samplesPerBlock;
        specStereo.sampleRate = sampleRate;
        specStereo.numChannels = 2;
             
        oversmpBuffer.setSize(2, samplesPerBlock * oversamplingFactor);
        oscillatorBuffer.setSize(2, samplesPerBlock);
        subBuffer.setSize(1, samplesPerBlock);
        noiseBuffer.setSize(1, samplesPerBlock);
        mixerBuffer.setSize(2, samplesPerBlock);
        modulation.setSize(2, samplesPerBlock);
        frequencyBuffer.setSize(1, samplesPerBlock * oversamplingFactor);
        
        antialiasingFilterL.reset();
        antialiasingFilterR.reset();
        halfBandCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(stereoOversampledSpec.sampleRate, jmin(20000.0, sampleRate * 0.4999));
        antialiasingFilterL.coefficients = halfBandCoeffs;
        antialiasingFilterR.coefficients = halfBandCoeffs;
        
		// initializing oscillators, noise generator and filters
        sawOscs.prepareToPlay(stereoOversampledSpec);
//        sawOscs.prepareToPlay(specStereo);
        subOscillator.prepareToPlay(sampleRate);
        noiseOsc.prepareToPlay(spec);
        noiseFilter.prepareToPlay(spec);
        moogFilter.prepareToPlay(sampleRate);
        lfo.prepareToPlay(sampleRate);
        ampAdsr.prepareToPlay(sampleRate);
        noteNumber.reset(sampleRate, 0.001f);
//        sawGainn.reset(sampleRate, 0.01f);
        masterGain.reset(sampleRate, 0.001f);
	}
    
    void updatePosition(AudioPlayHead::CurrentPositionInfo newPosition)
    {
        lfo.updatePosition(newPosition);
    }
    
    void updateFreqs()
    {
        double baseFreq = nn2hz(currentMidiNote) * std::pow(2, sawRegister + 1);
        double subFreq = baseFreq / std::pow(2, subRegister + 3);
        subOscillator.setFrequency(subFreq);
    }
    
//    void updateGain()
//    {
//        sawGainn.setTargetValue(sawGain * velocityLevel / std::sqrt(sawOscs.getActiveOscs()));
//    }
    
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
        sawGain = newValue;
//        sawGainn.setTargetValue(newValue);
//        updateGain();
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
        masterGain.setTargetValue(Decibels::decibelsToGain(newValue));
    }
    
private:
    dsp::ProcessSpec spec;
    dsp::ProcessSpec specStereo;
    dsp::ProcessSpec stereoOversampledSpec;
    
    int oversamplingFactor = 2;
    
    juce::dsp::IIR::Filter<float> antialiasingFilterL, antialiasingFilterR;
    juce::dsp::IIR::Coefficients<float>::Ptr halfBandCoeffs;
    
    SawOscillators sawOscs;
    NoiseOsc noiseOsc;
    NaiveOscillator subOscillator;
    NaiveOscillator lfo;
    
    // to track detune, register parameters on active note
    int currentMidiNote = 60;
    SmoothedValue<double, ValueSmoothingTypes::Linear> noteNumber;
    AudioBuffer<double> frequencyBuffer;

	// double ADSR
	MyADSR ampAdsr;
    
    // used for triggering the noise envelope
    bool trigger = false;

    // osc level params
    float sawGain;
    float subGain;
    float noiseGain;
//    SmoothedValue<float, ValueSmoothingTypes::Linear> sawGainn;
    SmoothedValue<float, ValueSmoothingTypes::Linear> masterGain;
    
    // osc params
    int sawRegister = 0;
    // sub params
    int subRegister;
    // modify: this is not necessary unless I need to set the initial parameter value in the interface
    //int subWaveform;
    
    // filters
    MoogFilters moogFilter;             // LPF for the whole synth
    StereoFilter noiseFilter;           // filter for noise
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
