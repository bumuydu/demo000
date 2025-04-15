#pragma once

#define MAX_SAW_OSCS 16

class SawOscillators
{
public:
    SawOscillators(){}
    ~SawOscillators(){}
    
    SawOscillators(int defaultSawReg, int defaultSawNum, int defaultDetune, float defaultPhase, float defaultStereoWidth)
    : sawRegister(defaultSawReg), activeOscs(defaultSawNum),  sawDetune(defaultDetune), sawPhase(defaultPhase), sawStereoWidth(defaultStereoWidth)
    {
        initializeOscillators();
    };
    
    void prepareToPlay(double sampleRate, int samplesPerBlock, dsp::ProcessSpec specInput)
    {
        spec = specInput;
        
        // Inizializzo l'oscillatore
        for (int i = 0; i < MAX_SAW_OSCS; ++i)
        {
            oscillators[i].prepare(spec);
        }
        
        // without this it wouldn't prepare the synth properly
        setActiveOscs(5);
        
        // NOISE
        // We prepare the release envelope generator of the noise osc
//        noiseEnvelope.setSize(1, samplesPerBlock);    // mono --> modify?
//        noiseEnvelope.clear();
//        egNoise.prepareToPlay(sampleRate);
    }
    
    void initializeOscillators()
    {
        for (int i = 0; i < MAX_SAW_OSCS; ++i)
        {
            oscillators[i].initialise([](float x) { return x / MathConstants<float>::pi; });
            oscillators[i].setFrequency(0, true);
        }
    }
    
    void startNote(int midiNoteNumber, float velocity)
    {
        // Reset phase for each oscillator
        for (int i = 0; i < MAX_SAW_OSCS; ++i)
        {
            oscillators[i].reset();
        }
        
        // storing currentMidiNote for parameter changes related to the frequency
        currentMidiNote = midiNoteNumber;
//        float baseFreq = MidiMessage::getMidiNoteInHertz(midiNoteNumber);
        //setSawFreqs(baseFreq);
        updateFreqs();
//        trigger = true;
    }
    
    void process(dsp::ProcessContextReplacing<float>& context)
    {
        for (int i = 0; i < activeOscs; ++i)
        {
            oscillators[i].process(context);
        }
    }
    
    // methods to calculate the frequencies of each oscillator
    
    void updateFreqs()
    {
        float baseFreq = MidiMessage::getMidiNoteInHertz(currentMidiNote) * std::pow(2, sawRegister);
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
    
    // osc params
    int sawRegister;
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
    
//    // We use the JUCE class Random to generate noise
//    Random noise;
//    bool trigger = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SawOscillators)
};

