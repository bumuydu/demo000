#pragma once
#include <JuceHeader.h>
#include "MyADSR.h"
#include "Matrix.h"
#include "PluginParameters.h"

class LadderFilter : public dsp::LadderFilter<float>
{
public:
    LadderFilter(){}
    ~LadderFilter(){}
    
    void prepare(const dsp::ProcessSpec& spec)
    {
        dsp::LadderFilter<float>::prepare(spec);
        
        // default values
        dsp::LadderFilter<float>::setMode(dsp::LadderFilter<float>::Mode::LPF12);
        dsp::LadderFilter<float>::setCutoffFrequencyHz(Parameters::defaultFiltHz);
        dsp::LadderFilter<float>::setResonance(0.0f);
    }
    
    void processWithEG(dsp::ProcessContextReplacing<float>& context, MyADSR& adsr, float lfoVal, int numSamples)
    {
//        auto mixerData = buffer.getArrayOfWritePointers();
//        dsp::AudioBlock<float> mixerBlock{ mixerData, 2, (size_t)numSamples };
//        dsp::ProcessContextReplacing<float> mixerContext{ mixerBlock };
        
//        if(egAmt == 0)
//        {
//            
//            dsp::AudioBlock<float> mixerBlock{ mixerData, 2, (size_t)numSamples };
//            dsp::ProcessContextReplacing<float> mixerContext{ mixerBlock };
//            process(mixerContext);
//        }
//        else
//        {
//            // must loop over channels and samples to process
//            for (int smp = 0; smp < numSamples; ++smp)
//            {
//                float env = adsr.getNextSample();
//                DBG("env val:" + String(env));
//                DBG("egAmt:" + String(egAmt));
//                DBG("cutoff:" + String(cutoff));
//
//                float modulatedCutoff = cutoff + (env * egAmt * 5000.0f);
//                DBG("MODDED:" + String(modulatedCutoff));
//                modulatedCutoff = jlimit(20.0f, 20000.0f, modulatedCutoff);
//                setCutoffFrequencyHz(modulatedCutoff);
//                
//                for (int ch = 0; ch < 2; ++ch)
//                {
//                    mixerData[ch][smp] = processSample(mixerData[ch][smp], ch);
//                }
//            }
//        }
        
        // working sample by sample was TOO CPU-heavy -- would completely become noise
        // again, using context instead of buffer or block
        
        float modulatedCutoff;
        
        float env = adsr.getNextSample();
//        modulatedCutoff = cutoff + (env * egAmt * 5000.0f);           // 5000 Hz seems audible to control these parameters
        modulatedCutoff = cutoff + (env * egAmt * 5000.0f) + (lfoAmt * lfoVal * 5000.0f);

        // the cutoff must stay inside the audible range
        modulatedCutoff = juce::jlimit(20.0f, 20000.0f, modulatedCutoff);
        setCutoffFrequencyHz(modulatedCutoff);
        process(context);
    }
    
    void setLfoAmt(const float newValue)
    {
        lfoAmt = newValue;
    }
    
    void setCutoff(const float newValue)
    {
        cutoff = newValue;
        setCutoffFrequencyHz(cutoff);
    }
    
    void setEnvAmt(const float newValue)
    {
        egAmt = newValue;
    }
    
private:
    float lfoAmt = 0;
    float egAmt = 0;
    float cutoff;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LadderFilter)
};

class MoogFilter {
public:
    MoogFilter(){};
    void prepareToPlay(double sr, int outputChannels)
    {
        sampleRate = sr;
        numOutputChannels = outputChannels;
        maxCutoffFrequency = sr * 0.499;
        update();
    };
//    void processWithEG(dsp::ProcessContextReplacing<float>& context, MyADSR& adsr, float lfoVal, int numSamples)
//    {
//        // working sample by sample was TOO CPU-heavy -- would completely become noise
//        // again, using context instead of buffer or block
//        
//        float modulatedCutoff;
//        
//        float env = adsr.getNextSample();
////        modulatedCutoff = cutoff + (env * egAmt * 5000.0f);           // 5000 Hz seems audible to control these parameters
//        modulatedCutoff = cutoff + (env * egAmt * 5000.0f) + (lfoAmt * lfoVal * 5000.0f);
//
//        // the cutoff must stay inside the audible range
//        modulatedCutoff = juce::jlimit(20.0f, 20000.0f, modulatedCutoff);
//        setCutoff(modulatedCutoff);
//        process(context);
//    }
    void process(AudioBuffer<float>& buffer, MyADSR& adsr, float lfoVal, int numSamples, int channel)
    {
        auto bufferData = buffer.getArrayOfWritePointers();
//        const int endSample = startSample + numSamples;
        float modulatedCutoff;
        
//        process(context);
        for (int smp = 0; smp < numSamples ; ++smp)
        {
            float env = adsr.getNextSample();
            modulatedCutoff = cutoff + (env * egAmt * 5000.0f) + (lfoAmt * lfoVal * 5000.0f);
            modulatedCutoff = juce::jlimit(20.0f, 20000.0f, modulatedCutoff);
            setCutoff(modulatedCutoff);
//            for (int ch = 0; ch < 2; ++ch)
//            {
//                bufferData[ch][smp] = processSample(bufferData[ch][smp]);
//            }
            bufferData[channel][smp] = processSample(bufferData[channel][smp]);
        }
    }
    float processSample(float x)
    {
        y = jacobianMatrix.newtonRaphson(x, s1, s2, s3, s4, k, g);

        v1 = g * saturationLUT(x - k * y[3]);
        v2 = g * saturationLUT(y[0] - y[1]);
        v3 = g * saturationLUT(y[1] - y[2]);
        v4 = g * saturationLUT(y[2] - y[3]);

        s1 = y[0] + v1;
        s2 = y[1] + v2;
        s3 = y[2] + v3;
        s4 = y[3] + v4;

        return y[3];
    };
    void setCutoff(const double newCutoffFrequencyHz)
    {
        cutoff = jmin(newCutoffFrequencyHz, maxCutoffFrequency);
        update();
    };
    void setResonance(float newResonance)
    {
        k = newResonance;
    };
    void setLfoAmt(const float newValue)
    {
        lfoAmt = newValue;
    }
    void setEnvAmt(const float newValue)
    {
        egAmt = newValue;
    }

private:
    void update()
    {
        g = std::tan(juce::MathConstants<double>::pi * cutoff / sampleRate);
        g = saturationLUT(g);
    };

    dsp::LookupTableTransform<float> saturationLUT{ [](float x) { return std::tanh(x); }, float(-5), float(5), 128 };
    Matrix jacobianMatrix;
    double sampleRate = 44100.0;
    double cutoff = Parameters::defaultFiltHz;
    double maxCutoffFrequency = 0.0;
    float k = Parameters::defaultFiltQ;
    float g = 0;
    float v1 = 0, v2 = 0, v3 = 0, v4 = 0;
    float s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    float out[4] = { 0, 0, 0, 0 };
    float* y = out;
    float lfoAmt = 0;
    float egAmt = 0;
    int numOutputChannels = 0;
};

class MoogFilters {
public:
    MoogFilters(){};
    void prepareToPlay(double sr, int outputChannels)
    {
        filterL.prepareToPlay(sr, 1);
        filterR.prepareToPlay(sr, 1);
    }
//    void processWithEG(dsp::ProcessContextReplacing<float>& context, MyADSR& adsr, float lfoVal, int numSamples)
//    {
//        // working sample by sample was TOO CPU-heavy -- would completely become noise
//        // again, using context instead of buffer or block
//
//        float modulatedCutoff;
//
//        float env = adsr.getNextSample();
////        modulatedCutoff = cutoff + (env * egAmt * 5000.0f);           // 5000 Hz seems audible to control these parameters
//        modulatedCutoff = cutoff + (env * egAmt * 5000.0f) + (lfoAmt * lfoVal * 5000.0f);
//
//        // the cutoff must stay inside the audible range
//        modulatedCutoff = juce::jlimit(20.0f, 20000.0f, modulatedCutoff);
//        setCutoff(modulatedCutoff);
//        process(context);
//    }
    void process(AudioBuffer<float>& buffer, MyADSR& adsr, float lfoVal, int numSamples)
    {
        filterL.process(buffer, adsr, lfoVal, numSamples, 0);
        filterR.process(buffer, adsr, lfoVal, numSamples, 1);
    }
    void setCutoff(const double newCutoffFrequencyHz)
    {
        filterL.setCutoff(newCutoffFrequencyHz);
        filterR.setCutoff(newCutoffFrequencyHz);
    }
    void setResonance(float newResonance)
    {
        filterL.setResonance(newResonance);
        filterR.setResonance(newResonance);
    }
    void setLfoAmt(const float newValue)
    {
        filterL.setLfoAmt(newValue);
        filterR.setLfoAmt(newValue);
    }
    void setEnvAmt(const float newValue)
    {
        filterL.setEnvAmt(newValue);
        filterR.setEnvAmt(newValue);
    }

private:
    MoogFilter filterL;
    MoogFilter filterR;
};

class ReleaseFilter
{
public:
    ReleaseFilter(double defaultRelease = 0.7)
        : release(defaultRelease)
    {}
    
    ~ReleaseFilter(){}
    
    void prepareToPlay(double sr)
    {
        sampleRate = sr;
        updateAlpha();
    }
    
    void processBlock(AudioBuffer<float>& buffer, const int startSample, const int numSamples)
    {
        auto bufferData = buffer.getWritePointer(0);
        const int endSample = numSamples + startSample;
        
//        for (int smp = startSample; smp < endSample; ++smp)
//        {
//            // explicit casting float buffer to double -- old school but compact code
//            envelope = jmax((double)bufferData[smp], envelope * alpha);
//            bufferData[smp] = (float)envelope;
//        }
        
        for (int smp = startSample; smp < endSample; ++smp)
        {
            if (envelope > 0.0001)
            {
                bufferData[smp] = (float)envelope;
                envelope *= alpha;
            }
            else
            {
                bufferData[smp] = 0.0f;
                envelope = 0.0;
            }
        }
        
    }
    
    void noteOn()
    {
        envelope = 1;
    }
    
    // check if the envelope has (nearly) finished
    bool isActive() const
    {
        return envelope > 0.001;
    }
    
    void setRelease(double newValue)
    {
        release = newValue;
        updateAlpha();
    }
    
private:
    void updateAlpha()
    {
        const auto n = jmax(1.0, release * sampleRate);
        // updated the alpha calculation 
        alpha = pow(0.001, 1.0 / n);
    }
    
    double release;
    double sampleRate = 1.0;
    double alpha = 0.0;
    double envelope = 0.0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReleaseFilter)
};

class StereoFilter
{
public:
    StereoFilter(){}
    
    ~StereoFilter(){}
    
    void prepareToPlay(/*double sr, */const dsp::ProcessSpec& spec)
    {
        for (int i = 0; i < numFilters; ++i)
        {
            filters[i].prepare(spec);
            filters[i].setResonance(0.2f);
        }
        filters[0].setMode(dsp::LadderFilter<float>::Mode::LPF12);
        filters[0].setCutoffFrequencyHz(spec.sampleRate * 0.499);
        filters[1].setMode(dsp::LadderFilter<float>::Mode::HPF12);
        filters[1].setCutoffFrequencyHz(10.0f);
        setFrequency(0.5);
    }
    
    void processBlock(AudioBuffer<float>& buffer, const int numSamples)
    {
        dsp::AudioBlock<float> block{ buffer.getArrayOfWritePointers(), 1, (size_t)numSamples };
        dsp::ProcessContextReplacing<float> context{ block };
                
        filters[0].process(context);
        filters[1].process(context);
    }
    
    void setFrequency(const float newValue)
    {
        if (newValue < 0.5f)
        {     // low-pass
            float frequency = juce::jmap(newValue, 0.0f, 0.5f, 80.0f, 18000.0f);
            filters[0].setCutoffFrequencyHz(frequency);
        }else
        {     // high-pass
            float frequency = juce::jmap(newValue, 0.5f, 1.0f, 0.1f, 800.0f);
            filters[1].setCutoffFrequencyHz(frequency);
        }
    }
    
    void setQuality(const double newValue)
    {
        for (int i = 0; i < numFilters; ++i)
        {
            filters[i].setResonance(newValue);
        }
    }
    
private:
    
    // one LP one HP filter
    static const int numFilters = 2;
    dsp::LadderFilter<float> filters[numFilters];
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoFilter)
};
