#pragma once
#include "Blit.h"
#include "PluginParameters.h"
#include "Filters.h"
#include "Tempo.h"

#define MAX_SAW_OSCS 16

class MoogOsc
{
public:
    MoogOsc() {}
    ~MoogOsc() {}

    void prepareToPlay(const dsp::ProcessSpec spec1)
    {
        sampleRate = spec1.sampleRate;
        blit.prepareToPlay(spec1);
    }
    
    void getNextAudioBlock(AudioBuffer<float>& outputBuffer, AudioBuffer<double>& frequencyBuffer,
                           int startSample, int numSamples)
    {
        int endSample = startSample + numSamples;
        for (int k = startSample; k < endSample; ++k)
        {
            double frequencySample = frequencyBuffer.getSample(0, k);
            const float sampleValue = getNextAudioSample(frequencySample);
            outputBuffer.addSample(0, k, sampleValue);
        }
    }
    
    float getNextAudioSample(double frequencySample)
    {
        sampleValue = blit.updateWaveform(frequencySample, waveform);
        return sampleValue;
    }
    
    void setWaveform(float newValue)
    {
        waveform = roundToInt(newValue);
    }
    
    void setOscPhase(const int newValue, const double frequency)
    {
        blit.setBlitPhase(newValue, frequency);
    }

    void clearAccumulator()
    {
        blit.clearAccumulator();
    }

private:
    Blit blit;
    double sampleRate = 44100.0;
    float sampleValue = 0.0f;
    int waveform = Parameters::defaultMainWf;
};

class SawOscillators
{
public:
    SawOscillators(){}
    ~SawOscillators(){}
    
    SawOscillators(int defaultSawNum, int defaultDetune, float defaultStereoWidth)
    : activeOscs(defaultSawNum),  sawDetune(defaultDetune), sawStereoWidth(defaultStereoWidth)
    {
    };
    
    void prepareToPlay(const dsp::ProcessSpec specInput)
    {
        spec = specInput;
        tmpPanBuffer.setSize(1, spec.maximumBlockSize);
        
        // Inizializzo l'oscillatore
        for (int i = 0; i < MAX_SAW_OSCS; ++i)
        {
            blitsOscs[i].prepareToPlay(specInput);
            frequencyBuffers[i].setSize(1, spec.maximumBlockSize);
        }
        
        setActiveOscs(Parameters::defaultSawNum);
    }
    
    void releaseResources()
    {
        tmpPanBuffer.setSize(0, 0);
        for (int i = 0; i < MAX_SAW_OSCS; ++i)
        {
            frequencyBuffers[i].setSize(0, 0);
        }
    }

    void startNote(const double freq)
    {
        // if phase resetting is ON then set it to the selected value by the user
        if(phaseResetting)
            setSawsPhase(sawPhase, freq);
    }
    
    // the process method now with stereo width parameter that pans every oscillator
    void process(AudioBuffer<float>& buffer, AudioBuffer<double>& frequencyBuffer,
                 const int startSampleOversampled, const int numSamplesOversampled)
    {
        auto* left = buffer.getWritePointer(0);
        auto* right = buffer.getWritePointer(1);
        
        setSawFreqs(frequencyBuffer, startSampleOversampled, numSamplesOversampled);
        
        for (int i = 0; i < activeOscs; ++i)
        {
            tmpPanBuffer.clear();
            
            float oscPosition;
            if (activeOscs == 1)
                oscPosition = 0.5f; // don't pan if activeOscs = 1
            else
                oscPosition = static_cast<float>(i) / static_cast<float>(activeOscs - 1);

            // since sawStereoWidth's range is [0.0f, 1.0f], we have to work around it to get the pan value right
            float pan = juce::jmap(sawStereoWidth, 0.5f, oscPosition);
            
            // equal power (constant power) panning law
            float leftGain  = std::cos(pan * juce::MathConstants<float>::halfPi);
            float rightGain = std::sin(pan * juce::MathConstants<float>::halfPi);

            // use tmpPanBuffer to process the oscillators
            // then add its contents to the main buffer applying the pan on buffers
            blitsOscs[i].getNextAudioBlock(tmpPanBuffer, frequencyBuffers[i],
                                           startSampleOversampled, numSamplesOversampled);
        
            const float* tmp = tmpPanBuffer.getReadPointer(0);
            const int endSampleOs = startSampleOversampled + numSamplesOversampled;
            for (int smp = startSampleOversampled; smp < endSampleOs; ++smp)
            {
                left[smp]  += tmp[smp] * leftGain;
                right[smp] += tmp[smp] * rightGain;
            }
        }
    }
    
    // methods to calculate the frequencies of each oscillator
    
    // depending on the number of saws and detune value
    // the frequencies of the rest of the oscillators have to be calculated
    // if numSaw odd --> then base is unchanged and the rest will be as before
    // if numSaw even --> then also the 1st oscillator's frequency will be detuned
    // freqBuffer and all frequencyBuffers are in the OVERSAMPLED buffer size
    void setSawFreqs(AudioBuffer<double>& freqBuffer, const int startSampleOversampled, const int numSamplesOversampled)
    {
        const int endSampleOversampled = startSampleOversampled + numSamplesOversampled;

        frequencyBuffers[0].copyFrom(0, startSampleOversampled, freqBuffer, 0, startSampleOversampled, numSamplesOversampled);

        if (activeOscs < 2)
            return;

        const bool isOdd = activeOscs % 2 != 0;
        const int numPairs = isOdd ? (activeOscs - 1) / 2 : activeOscs / 2;
        const int startIndex = isOdd ? 1 : 0;

        for (int i = startIndex; i < activeOscs; i += 2)
        {
            // 1,2,3... for each pair
            const int pairIndex = (i + 1) / 2;
            double detuneAmount;

            // just two active oscs needs special case
            if (!isOdd && numPairs == 1)
                detuneAmount = pow(cent, 0.5);
            else
                detuneAmount = pow(cent, static_cast<double>(pairIndex) / numPairs);

            for (int j = startSampleOversampled; j < endSampleOversampled; ++j)
            {
                const double sample = freqBuffer.getSample(0, j);
                frequencyBuffers[i].setSample(0, j, sample * detuneAmount);
                frequencyBuffers[i + 1].setSample(0, j, sample / detuneAmount);
            }
        }
    }
    
    // SETTERS, GETTERS AND MISC.
    
    void setWf(const int newValue)
    {
        for (int i = 0; i < MAX_SAW_OSCS; ++i)
        {
            blitsOscs[i].setWaveform(newValue);
        }
    }
    
    void setRegister(const int newValue)
    {
        // -2 because the newValue is the index of the AudioParameterChoice which isn't the actual value
        sawRegister = newValue - 2;
    }
    
    void setDetune(const float newValue)
    {
        sawDetune = newValue;
        cent = pow(root, newValue);
    }
    
    void setStereoWidth(const float newValue)
    {
        sawStereoWidth = newValue;
    }
    
    void setPhaseResetting(const bool newValue)
    {
        // if resetting is on, the phase value will be used on every startNote
        phaseResetting = newValue;
    }
    
    void setSawsPhase(const int phaseDegree, const double frequency)
    {
        sawPhase = phaseDegree;
        if (phaseDegree == 0) {
            for (int i = 0; i < MAX_SAW_OSCS; ++i)
            {
                blitsOscs[i].clearAccumulator();
            }
        }
        else
        {
            for (int i = 0; i < MAX_SAW_OSCS; ++i)
            {
                blitsOscs[i].clearAccumulator();
                blitsOscs[i].setOscPhase(phaseDegree, frequency);
            }
        }
    }
    
    void setPhaseDegree(const int newValue)
    {
        sawPhase = newValue;
    }
    
    void setActiveOscs(const int newValue)
    {
        activeOscs = newValue;
    }
    
    int getActiveOscs()
    {
        return activeOscs;
    }
    
private:
    dsp::ProcessSpec spec;

    MoogOsc blitsOscs[MAX_SAW_OSCS];
    int activeOscs;          // to obtain the JP8000 supersaw sound, 7 detuned oscillators must be used
    
    AudioBuffer<float> tmpPanBuffer;
    AudioBuffer<double> frequencyBuffers[MAX_SAW_OSCS];
    
    // osc params
    int sawRegister = -2;
    int sawDetune;
    bool phaseResetting = false;
    int sawPhase = 0;
    float sawStereoWidth;
    
    // calculating cents to detune
    // 1 cent is 1/1200 octave
    const double root = std::exp(std::log(2) / 1200);
    // raise to the number of cents (15 sounds nice)
    double cent = pow(root, 15);
    // to track the parameters of detune & register on active note
//    int currentMidiNote = 60;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SawOscillators)
};

class NoiseOsc
{
public:
    NoiseOsc(){}
    ~NoiseOsc(){}
    
    void releaseResources()
    {
        noiseEnvelope.setSize(0, 0);
    }
    
    void prepareToPlay(const dsp::ProcessSpec& spec)
    {
        // NOISE
        // We prepare the release envelope generator of the noise osc
        noiseEnvelope.setSize(1, spec.maximumBlockSize);
        noiseEnvelope.clear();
        egNoise.prepareToPlay(spec.sampleRate);
    }
    
    void trigger(int startSample, float velocity)
    {
//        noiseEnvelope.setSample(0, startSample, velocity);
//        velocityLevel = velocity;
        
        egNoise.noteOn();
        velocityLevel = velocity;
        
    }
    
    void process(AudioBuffer<float>& buffer, int startSample, int numSamples, float gain)
    {
//        buffer.clear();
//        noiseEnvelope.clear();
        if (velocityLevel <= 0.0001f || gain <= 0.0001f || !egNoise.isActive())
            return;
        auto noiseData = buffer.getArrayOfWritePointers();
        
        egNoise.processBlock(noiseEnvelope, startSample, numSamples);
        const auto numChannels = buffer.getNumChannels();
        for (int ch = 0; ch < numChannels; ++ch)
            for (int smp = 0; smp < numSamples; ++smp)
                noiseData[ch][smp] += velocityLevel * gain * ((noise.nextFloat() * 2.0f) - 1.0f);
        
        for (int ch = 0; ch < numChannels; ++ch)
        {
            FloatVectorOperations::multiply(noiseData[ch], noiseEnvelope.getReadPointer(0), numSamples);
        }
    }
    
    void setRelease(const float newValue)
    {
        egNoise.setRelease(newValue);
    }
    
    bool envFinished() const
    {
        return !egNoise.isActive();
    }
    
    
private:
    float velocityLevel = 0.7f;

    // We use the JUCE class Random to generate noise
    Random noise;
    AudioBuffer<float> noiseEnvelope;
    // envelope generator for the noise osc
    ReleaseFilter egNoise;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoiseOsc)
};


class NaiveOscillator {
public:
    NaiveOscillator(const double defaultFrequency, const int defaultWaveform)
    {
        frequency.setTargetValue(defaultFrequency);
        waveform = defaultWaveform;
    }

    ~NaiveOscillator(){}

    void prepareToPlay(const double sr)
    {
        frequency.reset(sr, 0.02);
        samplePeriod = 1.0 / sr;
        sampleRate = sr;
    }

    void setFrequency(const double newValue)
    {
        // no zero-frequency allowed
        jassert(newValue > 0);
        // important to put a comment over jassert for YOURSELF
        // because if it happens, in the debugger that comment is printed!!

        frequency.setTargetValue(newValue);
    }
    
    void setRate(const float newValue)
    {
        ppqPeriod = MetricTime::duration[roundToInt(newValue)].value * 4.0;
    }

    void setWaveform(const int newValue)
    {
        waveform = newValue;
    }
    
    void setSyncOn(const int newValue)
    {
        synced = newValue;
    }

//    float getNextAudioBlock(AudioBuffer<double>& buffer, const int numSamples)
// modify: delete this if it isn't being used
    float getNextAudioBlockOld(AudioBuffer<double>& buffer, const int startSample, const int numSamples)
    {
        const int numCh = buffer.getNumChannels();
        auto data = buffer.getArrayOfWritePointers();

        // numSamples - 1 because I return the last sample
        const int endSample = startSample + numSamples;
//        for (int smp = 0; smp < numSamples - 1; ++smp)
        for (int smp = startSample; smp < endSample - 1; ++smp)
        {
            const double sampleValue = getNextAudioSample();

            for (int ch = 0; ch < numCh; ++ch)
            {
                data[ch][smp] = sampleValue;
            }
        }
        return getNextAudioSample();
    }
    
    void getNextAudioBlock(AudioBuffer<double>& buffer, const int startSample, const int numSamples)
    {
        const int numCh = buffer.getNumChannels();
        auto data = buffer.getArrayOfWritePointers();

        const int endSample = startSample + numSamples;
        for (int smp = startSample; smp < endSample; ++smp)
        {
            const double sampleValue = getNextAudioSample();

            for (int ch = 0; ch < numCh; ++ch)
            {
                data[ch][smp] = sampleValue;
            }
        }
    }
    
    void getNextAudioBlockFloat(AudioBuffer<float>& buffer, const int startSample, const int numSamples)
    {
        auto data = buffer.getArrayOfWritePointers();

        const int endSample = startSample + numSamples;
        for (int smp = startSample; smp < endSample; ++smp)
            data[0][smp] = getNextAudioSample();
    }

    float getNextAudioSample()
    {
        auto sampleValue = 0.0;

        switch (waveform)
        {
        case 0: // sinusoidale
            sampleValue = sin(MathConstants<double>::twoPi * currentPhase);
            break;
        case 1: // triangular
            sampleValue = 4.0 * abs(currentPhase - 0.5) - 1.0;
            break;
        case 2: // saw UP
            sampleValue = 2.0 * currentPhase - 1.0;
            break;
        case 3: // square
            sampleValue = (currentPhase > 0.5) - (currentPhase < 0.5);
            break;
//        case 4: // Stepped S&H
//                sampleValue = ... ;
//        case 5: // Smooth S&H
//                sampleValue = ... ;
        default:
            // If it comes here, there is sth wrong
            // LFO Oscillator not selected correctly
            jassertfalse;
            break;
        }
        
        if(synced)
            updatePhaseSync();
        else
        {
            phaseIncrement = frequency.getNextValue() * samplePeriod;
            currentPhase += phaseIncrement;
            currentPhase -= static_cast<int>(currentPhase);
        }

        return sampleValue;
    }
    
    void updatePosition(AudioPlayHead::CurrentPositionInfo newPosition)
    {
        if (synced)
        {
            syncPhaseIncrement = ((newPosition.bpm / 60.0) / ppqPeriod) * samplePeriod;
            if (newPosition.isPlaying)
                currentPhase = fmod(newPosition.ppqPosition, ppqPeriod) / ppqPeriod;
        }
    }
    
    void updatePhaseSync()
    {
        nominalPhase += syncPhaseIncrement;
        nominalPhase -= static_cast<int>(nominalPhase);
    }
    
    void resetPhase()
    {
        currentPhase = 0;
    }


private:

    int waveform;
    bool synced = 0;

    // we do multiplicative this time but it must not be freq=0, put assert in setFreq()
    // questo, siccome e' un SmoothedValue va resettato nel prepareToPlay()
    SmoothedValue<double, ValueSmoothingTypes::Multiplicative> frequency;

    double currentPhase = 0;
    double phaseIncrement = 0;
    double samplePeriod = 1.0;
    double sampleRate = 0.0;
    double ppqPeriod = 0.0;
    double syncPhaseIncrement = 0.0;
    double nominalPhase = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NaiveOscillator)
};
