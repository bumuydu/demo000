/*
  ==============================================================================

    Matrix.cpp
    Created: 6 Dec 2021 8:31:49pm
    Author:  david

  ==============================================================================
*/

#include "Matrix.h"

float* Matrix::newtonRaphson(float in, float s1, float s2, float s3, float s4, float k, float g)
{
    cont = 0;
    input = in;
    residualVector[0] = -(g * saturationLUT(input - k * out[3] - out[0]) + s1 - out[0]);
    residualVector[1] = -(g * saturationLUT(out[0] - out[1]) + s2 - out[1]);
    residualVector[2] = -(g * saturationLUT(out[1] - out[2]) + s3 - out[2]);
    residualVector[3] = -(g * saturationLUT(out[2] - out[3]) + s4 - out[3]);
    norm = sqrt(pow(residualVector[0], 2) + pow(residualVector[1], 2) + pow(residualVector[2], 2) + pow(residualVector[3], 2));
   
    while (norm > threshold) {
        calculateJacobianMatrix(k, g);
        updateOutput();
        // The residual is negated for the minus in the formula
        residualVector[0] = -(g * saturationLUT(input - k * out[3] - out[0]) + s1 - out[0]);
        residualVector[1] = -(g * saturationLUT(out[0] - out[1]) + s2 - out[1]);
        residualVector[2] = -(g * saturationLUT(out[1] - out[2]) + s3 - out[2]);
        residualVector[3] = -(g * saturationLUT(out[2] - out[3]) + s4 - out[3]);
        norm = sqrt(pow(residualVector[0], 2) + pow(residualVector[1], 2) + pow(residualVector[2], 2) + pow(residualVector[3], 2));
        cont > 100 ? norm = threshold : cont++;
    };
    // The saturationLUT on the out is not necessary. If the next four line are commented, the self oscillation stage of the filter is incremented when the resonance is increase.
    // With this version, thank to the saturation tanh() the self oscillation is limited even if the resonance increase.
    out[0] = saturationLUT(out[0]);
    out[1] = saturationLUT(out[1]);
    out[2] = saturationLUT(out[2]);
    out[3] = saturationLUT(out[3]);

    return out;
};

void Matrix::calculateJacobianMatrix(float k, float g)
{
    // First row
    tempMatrix[0][0] = -g + g * pow(saturationLUT(out[0] + k * out[3] - input), 2.0) - 1;
    tempMatrix[0][3] = -g * k + g * k * pow(saturationLUT(out[0] + k * out[3] - input), 2.0);
    // Second row
    tempMatrix[1][0] = g - g * pow(saturationLUT(out[0] - out[1]), 2.0);
    tempMatrix[1][1] = -g + g * pow(saturationLUT(out[0] - out[1]), 2.0) - 1;
    // Third row
    tempMatrix[2][1] = g - g * pow(saturationLUT(out[1] - out[2]), 2.0);
    tempMatrix[2][2] = -g + g * pow(saturationLUT(out[1] - out[2]), 2.0) - 1;
    // Fourth row
    tempMatrix[3][2] = g - g * pow(saturationLUT(out[2] - out[3]), 2.0);
    tempMatrix[3][3] = -g + g * pow(saturationLUT(out[2] - out[3]), 2.0) - 1;
    inverse();
};

void Matrix::inverse()
{
    // Determinant using Sarrus
    float det = (tempMatrix[0][0] * tempMatrix[1][1] * tempMatrix[2][2] * tempMatrix[3][3]) - (tempMatrix[0][3] * tempMatrix[1][0] * tempMatrix[2][1] * tempMatrix[3][2]);
    det = 1 / det;

    // Computing the inverse matrix
    jacobianMatrix[0][0] = (tempMatrix[1][1] * tempMatrix[2][2] * tempMatrix[3][3]) * det;
    jacobianMatrix[1][0] = -(tempMatrix[1][0] * tempMatrix[2][2] * tempMatrix[3][3]) * det;
    jacobianMatrix[2][0] = (tempMatrix[1][0] * tempMatrix[2][1] * tempMatrix[3][3]) * det;
    jacobianMatrix[3][0] = -(tempMatrix[1][0] * tempMatrix[2][1] * tempMatrix[3][2]) * det;

    jacobianMatrix[0][1] = -(tempMatrix[0][3] * tempMatrix[2][1] * tempMatrix[3][2]) * det;
    jacobianMatrix[1][1] = (tempMatrix[0][0] * tempMatrix[2][2] * tempMatrix[3][3]) * det;
    jacobianMatrix[2][1] = -(tempMatrix[0][0] * tempMatrix[2][1] * tempMatrix[3][3]) * det;
    jacobianMatrix[3][1] = (tempMatrix[0][0] * tempMatrix[2][1] * tempMatrix[3][2]) * det;

    jacobianMatrix[0][2] = (tempMatrix[0][3] * tempMatrix[1][1] * tempMatrix[3][2]) * det;
    jacobianMatrix[1][2] = -(tempMatrix[0][3] * tempMatrix[1][0] * tempMatrix[3][2]) * det;
    jacobianMatrix[2][2] = (tempMatrix[0][0] * tempMatrix[1][1] * tempMatrix[3][3]) * det;
    jacobianMatrix[3][2] = -(tempMatrix[0][0] * tempMatrix[1][1] * tempMatrix[3][2]) * det;

    jacobianMatrix[0][3] = -(tempMatrix[0][3] * tempMatrix[1][1] * tempMatrix[2][2]) * det;
    jacobianMatrix[1][3] = (tempMatrix[0][3] * tempMatrix[1][0] * tempMatrix[2][2]) * det;
    jacobianMatrix[2][3] = -(tempMatrix[0][3] * tempMatrix[1][0] * tempMatrix[2][1]) * det;
    jacobianMatrix[3][3] = (tempMatrix[0][0] * tempMatrix[1][1] * tempMatrix[2][2]) * det;
};

void Matrix::updateOutput()
{
    out[0] += jacobianMatrix[0][0] * residualVector[0] + jacobianMatrix[0][1] * residualVector[1] + jacobianMatrix[0][2] * residualVector[2] + jacobianMatrix[0][3] * residualVector[3];
    out[1] += jacobianMatrix[1][0] * residualVector[0] + jacobianMatrix[1][1] * residualVector[1] + jacobianMatrix[1][2] * residualVector[2] + jacobianMatrix[1][3] * residualVector[3];
    out[2] += jacobianMatrix[2][0] * residualVector[0] + jacobianMatrix[2][1] * residualVector[1] + jacobianMatrix[2][2] * residualVector[2] + jacobianMatrix[2][3] * residualVector[3];
    out[3] += jacobianMatrix[3][0] * residualVector[0] + jacobianMatrix[3][1] * residualVector[1] + jacobianMatrix[3][2] * residualVector[2] + jacobianMatrix[3][3] * residualVector[3];
};




