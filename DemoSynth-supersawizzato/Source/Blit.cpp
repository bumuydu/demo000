/*
  ==============================================================================

    Blit.cpp
    Created: 16 Oct 2021 11:20:00am
    Author:  david

  ==============================================================================
*/

#include "Blit.h"

void Blit::prepareToPlay(double sr) {
    this->sr = sr;
    sp = 1.0 / sr;

    alpha = exp(-(LEAKY_INTEGRATOR_BASE_FREQUENCY / sr) * MathConstants<double>::twoPi);

    leakMod   = alpha - exp(-(LEAKY_INTEGRATOR_MOD_FREQUENCY/sr) * MathConstants<double>::twoPi);
    leakNoise = alpha - exp(-(LEAKY_INTEGRATOR_NOISE_FREQUENCY/sr) * MathConstants<double>::twoPi);
    leakTrNoi = alpha - exp(-(LEAKY_INTEGRATOR_TRI_NOISE_FREQUENCY/sr) * MathConstants<double>::twoPi);
    leakTrMod = alpha - exp(-(LEAKY_INTEGRATOR_TRI_MOD_FREQUENCY / sr) * MathConstants<double>::twoPi);

    populateBlitTab();
}

void Blit::populateBlitTab()
{
    double step = 0.001; // 1/1000
    double temp = 0.0;
    double totalSum = 0.0;

    for (int i = 0; i < 1000; i++) {
        totalSum = 0.0;
        temp = step * i;

        for (int x = 0; x < 32; x++) {
            blitsMatrix[i][x] = x == 16 ?
                (!temp ? (2.0 * M_PI * 0.45) : (sin(2.0 * M_PI * 0.45 * (-temp)) / (-temp))) :
                (sin(2.0 * M_PI * 0.45 * (x - 16.0 - temp)) / (x - 16.0 - temp));
            blitsMatrix[i][x] *= (0.51 - 0.49 * (cos(2.0 * M_PI * (x - temp) / 32.0)));

            totalSum += blitsMatrix[i][x];
        }

        // Normalize (Sum of all sample = 1)
        for (int x = 0; x < 32; x++) {
            blitsMatrix[i][x] /= totalSum;
        }
    }
}

void Blit::updateLeakiness(double osc3LfoRate, double modAmount, bool isOscModOn, bool isNoiseModOn, double modMix) 
{
    // V3
    const double noiseAmt = pow(modMix, 1.5) * isNoiseModOn;
    leakiness    = leakNoise * noiseAmt;
    leakinessTri = leakTrNoi * noiseAmt;

    const double oshModFactor = jmin(osc3LfoRate * 0.004, 1.0); // * 0.004 = / 250
    leakiness    += oshModFactor * oshModFactor * leakMod;
    leakinessTri += oshModFactor * oshModFactor * leakTrMod;

    const double totModAmount = modAmount * isOscModOn;
    leakiness    *= totModAmount;
    leakinessTri *= totModAmount;

    // Actual leakyness = alpha - leakiness (or alpha - leakinessTri for tri wave)
}

void Blit::clearAccumulator() {
    accTri = 0.0;
    accSaw = 0.0;
    accSquare = 0.0;
    sampleCont = 0;
}

float Blit::updateWaveform(double f, int waveform)
{
    float tempSample = 0.0f;
    decrementStep = f * sp;

    switch (waveform) {
        case 0:
            tempSample = updateSawDown(f);
            break;
        case 1:
            tempSample = updateSawTri(f);
            break;
        case 2:
            tempSample = updateTriangle(f);
            break;
        case 3:
            tempSample = updateSawUp(f);
            break;
        case 4:
            tempSample = updateSquare(f);
            break;
        case 5:
            tempSample = updateWideSquare(f);
            break;
        case 6:
            tempSample = updateNarrowSquare(f);
            break;
        default:
            break;
    }

    pBlit[index] = 0.0;
    nBlit[index] = 0.0;
    sampleCont++;
    index++;
    return tempSample;
}

float Blit::updateTriangle(double f) {
    accTri = accTri * (alpha - leakinessTri) + updateSquare(f) * 8.0 * f * sp;
    return accTri;
}

float Blit::updateSawTri(double f) {
    //float sawSample = updateSawDown(f);
    //float triSample = -updateTriangle(f);
    //return triSample * 0.7f + sawSample * 0.3f;
    const float triSample = updateTriangle(f);  // tri prima di saw perchè tri chiama square, che setta passedNeg, oppure mettere passNeg anche in saw
    const float sawSample = updateSawDown(f);
    return sawSample * 0.3f - triSample * 0.7f;
}

float Blit::updateSawDown(double f) {
    pEdge = sr / f + subOff1;
    if (sampleCont >= int(pEdge))
    {
        sampleCont = 0;
        getPositiveBlit();
        //decrementStep = f * sp;
    }
    accSaw = accSaw * (alpha - leakiness) + pBlit[index] - decrementStep;
    return accSaw;
}

float Blit::updateSawUp(double f) {
    nEdge = sr / f + subOff2;
    if (sampleCont >= int(nEdge))
    {
        sampleCont = 0;
        getNegativeBlit();
        //decrementStep = -1.0 * f * sp;
    }
    accSaw = accSaw * (alpha - leakiness) + nBlit[index] + decrementStep; // - decrementStep;
    return accSaw;
}

float Blit::updateSquare(double f) {
    pEdge = sr / f + subOff1;
    nEdge = (pEdge + subOff1) * 0.5;
    if (sampleCont >= int(pEdge))
    {
        passedNeg = false;
        sampleCont = 0;
        getPositiveBlit();
    }
    //if (sampleCont == int(nEdge)) getNegativeBlit();
    if (crossingNegEdge()) getNegativeBlit();
    accSquare = accSquare * (alpha - leakiness) + pBlit[index] + nBlit[index];
    //accSquare = pBlit[index] + nBlit[index];
    return accSquare;
}

float Blit::updateWideSquare(double f) {
    pEdge = sr / f + subOff1;
    nEdge = pEdge * 0.65 + subOff1 * 0.35;
    if (sampleCont >= int(pEdge))
    {
        passedNeg = false;
        sampleCont = 0;
        getPositiveBlit();
        // offset = -0.15;
    }
    //if (sampleCont == int(nEdge)) getNegativeBlit();
    if (crossingNegEdge()) getNegativeBlit();
    accSquare = accSquare * (alpha - leakiness) + pBlit[index] + nBlit[index];
    return accSquare - 0.0096; // - offset;
}

float Blit::updateNarrowSquare(double f) {
    pEdge = sr / f + subOff1;
    nEdge = pEdge * 0.8 + subOff1 * 0.2;
    if (sampleCont >= int(pEdge))
    {
        passedNeg = false;
        sampleCont = 0;
        getPositiveBlit();
        //offset = -0.30;
    }
    //if (sampleCont == int(nEdge)) getNegativeBlit();
    if (crossingNegEdge()) getNegativeBlit();
    accSquare = accSquare * (alpha - leakiness) + pBlit[index] + nBlit[index];
    return accSquare - 0.01522; // - offset;;
}



void Blit::getPositiveBlit() {
    int blitIndex = 0;
    subOff1 = pEdge - int(pEdge);
    blitIndex = subOff1 * 1000;
    unsigned char j = index;
    const double* blit = blitsMatrix[blitIndex];
    for (int i = 0; i < 32; ++i) {
        pBlit[j] += blit[i];
        j++;
    }
}

void Blit::getNegativeBlit() {
    int blitIndex = 0;
    subOff2 = nEdge - int(nEdge);
    blitIndex = subOff2 * 1000;
    unsigned char j = index;
    const double* blit = blitsMatrix[blitIndex];
    for (int i = 0; i < 32; ++i) {
        nBlit[j] -= blit[i];
        j++;
    }
}

bool Blit::crossingNegEdge()
{
    const int threshold = int(nEdge);
    const bool cross = !passedNeg && (sampleCont >= threshold);
    if (cross) passedNeg = true;
    return cross;
}
