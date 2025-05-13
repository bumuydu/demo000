/*
  ==============================================================================

    Matrix.h
    Created: 6 Dec 2021 8:31:49pm
    Author:  david

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
//#include "PluginParameters.h"


class Matrix {
public:
    Matrix() {};
    float* newtonRaphson(float in, float s1, float s2, float s3, float s4, float k, float g);

private:
    dsp::LookupTableTransform<float> saturationLUT{ [](float x) { return std::tanh(x); }, float(-5), float(5), 128 };
    float jacobianMatrix[4][4] = { 0 };
    float tempMatrix[4][4] = { 0 };
    float residualVector[4] = { 0 };
    float out[4] = { 0 };
    const float threshold = 0.000001f;
    int cont = 0;
    float input = 0;
    float norm = 0;
    void inverse();
    void calculateJacobianMatrix(float k, float g);
    void updateOutput();
};

