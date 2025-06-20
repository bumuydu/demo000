/*
  ==============================================================================

    Blit.h
    Created: 16 Oct 2021 11:11:45am
    Author:  david

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

#define LEAKY_INTEGRATOR_BASE_FREQUENCY        8.0

class Blit {
public:
    Blit() {}
    ~Blit() {}
    void prepareToPlay(const dsp::ProcessSpec spec);
    float updateWaveform(double frequencySample, int waveform);
    void setBlitPhase(const int phaseDegree, const double frequency);
    void clearAccumulator();

private:
    int maxBlock;
    double sr;
    double sp;
    double pEdge = 0.0;
    double nEdge = 0.0;
    double subOff1 = 0.0;
    double subOff2 = 0.0;
    
    double alpha = 0.999;
//    double leakiness = 0.0;
//    double leakinessTri = 0.0;
    double leakiness = 0.0001;
    double leakinessTri = 0.0001;

    double accSaw = 0.0;
    double accSquare = 0.0;
    double accTri = 0.0;

    double decrementStep = 0.0;
    double offset = 0.0;
    
    double mpi = MathConstants<double>::pi;

    int sampleCont = 0;
    bool passedNeg = false;
    unsigned char index = 0;

    double pBlit[256] = { 0 };
    double nBlit[256] = { 0 };
    double blitsMatrix[1000][32] = { 0 };
    
    void populateBlitTab();
    void getNegativeBlit();
    void getPositiveBlit();

    float updateTriangle(double frequencySample);
    float updateSawTri(double frequencySample);
    float updateSawUp(double frequencySample);
    float updateSawDown(double frequencySample);
    float updateSquare(double frequencySample);
    float updateWideSquare(double frequencySample);
    float updateNarrowSquare(double frequencySample);

    bool crossingNegEdge();
};
