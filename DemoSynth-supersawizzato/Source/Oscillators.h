#pragma once
#include "Filters.h"
#include "Tempo.h"

#define MAX_SAW_OSCS 16

class SawOscillators
{
public:
    SawOscillators(){}
    ~SawOscillators(){}
    
    SawOscillators(int defaultSawNum, int defaultDetune, float defaultPhase, float defaultStereoWidth)
    : activeOscs(defaultSawNum),  sawDetune(defaultDetune), sawPhase(defaultPhase), sawStereoWidth(defaultStereoWidth)
    {
        initializeOscillators();
    };
    
    void prepareToPlay(const dsp::ProcessSpec specInput)
    {
        spec = specInput;
        tmpPanBuffer.setSize(2, spec.maximumBlockSize);
        
        // Inizializzo l'oscillatore
        for (int i = 0; i < MAX_SAW_OSCS; ++i)
        {
            oscillators[i].prepare(spec);
        }
        
        // without this it wouldn't prepare the synth properly
        setActiveOscs(5);
    }
    
    void initializeOscillators()
    {
        for (int i = 0; i < MAX_SAW_OSCS; ++i)
        {
            oscillators[i].initialise([](float x) { return x / MathConstants<float>::pi; });
            oscillators[i].setFrequency(0, true);
        }
    }
    
    void startNote(int midiNoteNumber)
    {
        // Reset phase for each oscillator
        for (int i = 0; i < MAX_SAW_OSCS; ++i)
        {
            oscillators[i].reset();
        }
        
        // storing currentMidiNote for parameter changes related to the frequency
        currentMidiNote = midiNoteNumber;
        updateFreqs();
    }
    
//    void process(dsp::ProcessContextReplacing<float>& context)
//    {
//        for (int i = 0; i < activeOscs; ++i)
//        {
//            oscillators[i].process(context);
//        }
//    }
    
    // the process method now with stereo width parameter that pans every oscillator
    void processStereo(dsp::AudioBlock<float>& block, int numSamples)
    {
        auto* left = block.getChannelPointer(0);
        auto* right = block.getChannelPointer(1);

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
                // check if this works, try using startSample as well as numSamples
                // if not, try using NaiveOscillator and process on the buffer rather than the audiocontext
            dsp::AudioBlock<float> tmpPanBlock(tmpPanBuffer.getArrayOfWritePointers(), 2, numSamples);
            dsp::ProcessContextReplacing<float> tmpPanContext(tmpPanBlock);

            oscillators[i].process(tmpPanContext);

            const float* tmpL = tmpPanBlock.getChannelPointer(0);
            const float* tmpR = tmpPanBlock.getChannelPointer(1);

            for (int smp = 0; smp < numSamples; ++smp)
            {
                left[smp]  += tmpL[smp] * leftGain;
                right[smp] += tmpR[smp] * rightGain;
            }
        }
    }
    
    // methods to calculate the frequencies of each oscillator
    
    void updateFreqs()
    {
        float baseFreq = MidiMessage::getMidiNoteInHertz(currentMidiNote) * std::pow(2, sawRegister + 1);
        setSawFreqs(baseFreq);
    }
    
    void setSawFreqs(double baseFreq)
    {
        // depending on the number of saws and detune value
        // the frequencies of the rest of the oscillators have to be calculated
        // 1. if numSaw odd --> then base is unchanged and the rest will be as before
        //    if numSaw even --> then also the 1st oscillator's frequency will be detuned
        // 2.
        
        if (activeOscs % 2 != 0)
        {   // odd
            oscillators[0].setFrequency(baseFreq, true);
            
            if (activeOscs > 1)
            {
                for (int i = 1; i < activeOscs; i+=2)
                {
                    // 1,2,3... for each pair
                    int pairIndex = (i + 1) / 2;
                    double detuneAmount = pow(cent, pairIndex);
                    
                    oscillators[i].setFrequency(baseFreq * detuneAmount, true);
                    oscillators[i+1].setFrequency(baseFreq / detuneAmount, true);
                }
            }
        }
        else
        {   // even
            for (int i = 0; i < activeOscs; i+=2) {
                int pairIndex = (i + 1) / 2;
                double detuneAmount = pow(cent, pairIndex);
                oscillators[i].setFrequency(baseFreq * detuneAmount, true);
                oscillators[i+1].setFrequency(baseFreq / detuneAmount, true);
            }
        }
    }
    
    // setters & getters
    
    void setRegister(const int newValue)
    {
        // -2 because the newValue is the index of the AudioParameterChoice which isn't the actual value
        sawRegister = newValue - 2;
        updateFreqs();
    }
    
    void setDetune(const float newValue)
    {
        sawDetune = newValue;
        cent = pow(root, newValue);
        updateFreqs();
    }
    
    void setStereoWidth(const float newValue)
    {
        sawStereoWidth = newValue;
    }
    
    void setPhase(const float newValue)
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

    dsp::Oscillator<float> oscillators[MAX_SAW_OSCS];
    int activeOscs; // to obtain the JP8000 supersaw sound, 7 detuned oscillators must be used
    
    AudioBuffer<float> tmpPanBuffer;
    
    // osc params
    int sawRegister = -2;
    int sawDetune;
    int sawPhase;
    float sawStereoWidth;
    
    // calculating cents to detune
    // 1 cent is 1/1200 octave
    const double root = std::exp(std::log(2) / 1200);
    // raise to the number of cents (15 sounds nice)
    double cent = pow(root, 15);
    // to track the parameters of detune & register on active note
    int currentMidiNote = 60;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SawOscillators)
};

class NoiseOsc
{
public:
    NoiseOsc(){}
    ~NoiseOsc(){}
    
//    NoiseOsc()
//    :
//    {
//    };
    
    void releaseResources()
    {
        noiseEnvelope.setSize(0, 0);
    }
    
    void prepareToPlay(const dsp::ProcessSpec& spec/*double sampleRate, int samplesPerBlock*/)
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
    
    // old MONO method -- modify: must erase it if it is not going to be used
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
    
    // setters & getters
    
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

//    // We use the JUCE class Random to generate noise
    Random noise;
    AudioBuffer<float> noiseEnvelope;
    // envelope generator for the noise osc
    ReleaseFilter egNoise;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoiseOsc)
};


class NaiveOscillator {
public:
    NaiveOscillator(const double defaultFrequency = 20.0, const int defaultWaveform = 0)
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

    float getNextAudioBlock(AudioBuffer<double>& buffer, const int numSamples)
    {
        const int numCh = buffer.getNumChannels();
        auto data = buffer.getArrayOfWritePointers();

        // numSamples - 1 because I return the last sample
        for (int smp = 0; smp < numSamples - 1; ++smp)
        {
            const double sampleValue = getNextAudioSample();

            for (int ch = 0; ch < numCh; ++ch)
            {
                data[ch][smp] = sampleValue;
            }
        }
        return getNextAudioSample();
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
