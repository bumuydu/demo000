/*
  ==============================================================================

    SupersawTheme.h
    Created: 24 May 2025 1:46:00pm
    Author:  Derin Donmez

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

#define KNOB_SCALE 0.85
#define BORDER_WIDTH 1.5

class SupersawLookAndFeel : public LookAndFeel_V4
{
public:
    SupersawLookAndFeel()
    {
        //setColour(Slider::thumbColourId, Colours::red);
    }
    ~SupersawLookAndFeel() {}

    // prendo tutto questo dal documentazione. metto override e il nome slider
    void drawRotarySlider(Graphics& g, int x, int y, int width, int height,
        float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, Slider& slider) override
    {
        // calcolo alcune utili variabili
        const float radius = jmin(width, height) * 0.5f * KNOB_SCALE - BORDER_WIDTH * 0.5;
        const float centreX = x + width * 0.5;
        const float centreY = y + height * 0.5;

        const float knobX = centreX - radius;
        const float knobY = centreY - radius;
        const float knobW = radius * 2.0;

        const Colour brightCol = Colour(0xff2b2d31);
        const Colour darkCol = Colour(0xff0d0d11);

        // disegno il corpo della rotella
        g.setColour(Colours::darkblue);
        g.fillEllipse(knobX, knobY, knobW, knobW);

        g.setColour(Colours::white);
        g.drawEllipse(knobX, knobY, knobW, knobW, BORDER_WIDTH);

        // disegno il puntatore della rotella
        const auto pointerLen = radius * 0.33;
        const auto pointerThickness = 2.0;

        Path p;
        // rect koordinatlari. x: ortaya koymamiz icin pointerthickness'in yarisini cikarmamiz lazim
        // y: tepeden yarim radius cikaricaz
        p.addRectangle(pointerThickness * -0.5, -radius, pointerThickness, pointerLen);
        const float angle = jmap(sliderPosProportional, rotaryStartAngle, rotaryEndAngle);
        // in JUCE possiamo associare delle trasformazioni lineari ad ogni componente grafico (anche il g)
        // rotate by angle but not from the upper left corner, from the center of my rotary slider -> translated
        p.applyTransform(AffineTransform::rotation(angle).translated(centreX, centreY));

        g.setColour(Colours::white);
        g.fillPath(p);
        // disegno i tick -- lasciato come compito
    }

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SupersawLookAndFeel)
};
