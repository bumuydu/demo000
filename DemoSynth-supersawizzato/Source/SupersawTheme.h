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
        const int padding = width / 10;

        // Shrink the drawing area to add padding
        x += padding;
        y += padding;
        width -= 2 * padding;
        height -= 2 * padding;

        const float minRadius = 10.0f;
        const float maxRadius = juce::jmin(width / 2.0f, height / 2.0f) - 4.0f;
        const float radius = juce::jmax(minRadius, maxRadius);

        const float centreX = x + width * 0.5f;
        const float centreY = y + height * 0.5f;

        const float knobX = centreX - radius;
        const float knobY = centreY - radius;
        const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // shadow to slightly lower right side
        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.fillEllipse(knobX + 2.0f, knobY + 2.5f, radius * 2.0f, radius * 2.0f);

        // body
        g.setColour(juce::Colour(0xFF1A1A1A));
        g.fillEllipse(knobX, knobY, radius * 2.0f, radius * 2.0f);

        // top face
        const float topInset = juce::jmin(40.0f, radius * 0.4f);
        const float faceRadius = radius - topInset;
        if (faceRadius > 0.0f)
        {
            juce::ColourGradient faceGradient(
                juce::Colour(0xFFCCCCCC), centreX, centreY - faceRadius * 0.6f,
                juce::Colour(0xFF999999), centreX, centreY + faceRadius * 0.6f,
                false);
            g.setGradientFill(faceGradient);
            g.fillEllipse(centreX - faceRadius, centreY - faceRadius, faceRadius * 2.0f, faceRadius * 2.0f);
        }

        // indicator: white body line
        juce::Path whiteLine;
        float lineLength = radius;
        float lineThickness = 1.5f;
        whiteLine.addRectangle(-lineThickness * 0.5f, -lineLength, lineThickness, lineLength);
        g.setColour(juce::Colour(0xFFf0f1f1));
        g.fillPath(whiteLine, juce::AffineTransform::rotation(angle).translated(centreX, centreY));

        // indicator: black top line
        juce::Path blackPointer;
        float pointerLength = faceRadius;
        float pointerThickness = 3.0f;
        blackPointer.addRectangle(-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength);
        g.setColour(juce::Colours::black);
        g.fillPath(blackPointer, juce::AffineTransform::rotation(angle).translated(centreX, centreY));

        // outline
        g.setColour(juce::Colours::darkgrey);
        g.drawEllipse(knobX, knobY, radius * 2.0f, radius * 2.0f, 1.0f);

        // ticks
        int numTicks = 22;
        float tickRadius = radius + 4.0f;
        float tickLength = 4.0f;
        float tickWidth = 1.8f;

        float tickStartAngle = juce::MathConstants<float>::pi * 0.70f;
        float tickEndAngle = juce::MathConstants<float>::pi * 2.30f;

        g.setColour(juce::Colours::lightgrey);

        for (int i = 0; i < numTicks; ++i)
        {
            float proportion = i / float(numTicks - 1);
            float tickAngle = tickStartAngle + proportion * (tickEndAngle - tickStartAngle);
            float sinA = std::sin(tickAngle);
            float cosA = std::cos(tickAngle);

            float startX = centreX + cosA * tickRadius;
            float startY = centreY + sinA * tickRadius;
            float endX = centreX + cosA * (tickRadius + tickLength);
            float endY = centreY + sinA * (tickRadius + tickLength);

            g.drawLine(startX, startY, endX, endY, tickWidth);
        }
    }

    
    void drawLinearSlider (Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           Slider::SliderStyle, Slider& slider) override
    {
        const float tickPaddingLeft = 8.0f;   // left padding for ticks
        const float tickPaddingRight = 8.0f;  // right padding for symmetry
        const float verticalPadding = 6.0f;    // top/bottom padding for ticks and handle

        // Adjusted drawing area
        const float fx = static_cast<float>(x) + tickPaddingLeft;
        const float fy = static_cast<float>(y) + verticalPadding;
        const float fwidth = static_cast<float>(width) - tickPaddingLeft - tickPaddingRight;
        const float fheight = static_cast<float>(height) - 2.0f * verticalPadding;

        const float sectionWidth = fwidth / 3.0f;
        const float trackX = fx + sectionWidth;
        const float trackWidth = sectionWidth;
        const float trackY = fy;
        const float trackHeight = fheight;

        // Shadow color used consistently
        const juce::Colour shadowColour = juce::Colours::black.withAlpha(0.2f);

        // --- Draw left shadow/tick background
        g.setColour(shadowColour);
        g.fillRect(static_cast<float>(x), fy, tickPaddingLeft, fheight);

        // --- Draw right shadow/tick background
        g.setColour(shadowColour);
        g.fillRect(static_cast<float>(x) + width - tickPaddingRight, fy, tickPaddingRight, fheight);

        // --- Draw top shadow padding
        g.setColour(shadowColour);
        g.fillRect(static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), verticalPadding);

        // --- Draw bottom shadow padding
        g.setColour(shadowColour);
        g.fillRect(static_cast<float>(x), static_cast<float>(y) + height - verticalPadding, static_cast<float>(width), verticalPadding);

        
        // --- Draw ticks (22 total) on the left side
        const int numTicks = 22;
        const float tickLength = 6.0f;
        const float tickX = static_cast<float>(x) + 0.1f;

        g.setColour(juce::Colour(0xFFf0f1f1)); // better white

        for (int i = 0; i < numTicks; ++i)
        {
            float proportion = static_cast<float>(i) / (numTicks - 1);
            float yPos = fy + (1.0f - proportion) * fheight;
            g.drawLine(tickX, yPos, tickX + tickLength, yPos, 1.8f);
        }

        // --- Draw left shadow panel
        g.setColour(shadowColour);
        g.fillRect(fx, fy, sectionWidth, fheight);

        // --- Draw right shadow panel
        g.fillRect(fx + 2 * sectionWidth, fy, sectionWidth, fheight);

        // --- Draw center track outer (dark gray)
        g.setColour(juce::Colour(0xFF242525));
        g.fillRect(trackX, trackY, trackWidth, trackHeight);

        // --- Draw inner black track
        const float innerMargin = 2.0f;
        const float innerTrackWidth = static_cast<float>(width) / 3.0f - 2.0f * innerMargin;

        g.setColour(juce::Colours::black);
        g.fillRect((static_cast<float>(x) + width * 0.5f) - innerTrackWidth * 0.5f,
                   trackY + innerMargin,
                   innerTrackWidth,
                   trackHeight - 2.0f * innerMargin);
//
//        g.setColour(juce::Colours::black);
//        g.fillRect(trackX + innerMargin,
//                   trackY + innerMargin,
//                   trackWidth - 2 * innerMargin,
//                   trackHeight - 2 * innerMargin);

        // --- Draw handle
        const float handleWidth = fwidth;
        const float blackBarHeight = 8.0f;
        const float whiteLineHeight = 5.0f;
        const float handleTotalHeight = blackBarHeight * 2 + whiteLineHeight;

        const float clampedSliderPos = juce::jlimit(
            fy - handleTotalHeight * 0.5f,
            fy + fheight - handleTotalHeight * 0.5f,
            sliderPos - handleTotalHeight * 0.5f
        );

        const float handleTop = clampedSliderPos;
        const float centreX = fx + fwidth * 0.5f;

        // Top black rectangle
        g.setColour(juce::Colours::black);
        g.fillRect(centreX - handleWidth * 0.5f, handleTop, handleWidth, blackBarHeight);

        // White middle line
        g.setColour(juce::Colour(0xFFf0f1f1));
        g.fillRect(centreX - handleWidth * 0.5f,
                   handleTop + blackBarHeight,
                   handleWidth,
                   whiteLineHeight);

        // Bottom black rectangle
        g.setColour(juce::Colours::black);
        g.fillRect(centreX - handleWidth * 0.5f,
                   handleTop + blackBarHeight + whiteLineHeight,
                   handleWidth,
                   blackBarHeight);
    }


    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();

        // outer rounded rectangle
        const float cornerSize = 6.0f;
        juce::Rectangle<float> outerRect = bounds.reduced(2.0f);

        g.setColour(juce::Colour(0xFF1A1A1A));
        g.fillRoundedRectangle(outerRect, cornerSize);

        // draw border
        g.setColour(juce::Colours::darkgrey);
        g.drawRoundedRectangle(outerRect, cornerSize, 1.2f);

        // LED circle in the center
        auto ledRadius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 4.0f;
        juce::Point<float> center = bounds.getCentre();
        juce::Rectangle<float> ledBounds(center.x - ledRadius, center.y - ledRadius, ledRadius * 2.0f, ledRadius * 2.0f);
        
        // glow
        if (button.getToggleState())
        {
            juce::Colour glowColour = juce::Colour::fromRGBA(255, 0, 0, 40);
            for (int i = 1; i <= 3; ++i)
            {
                float glowScale = 1.0f + i * 0.4f;
                juce::Rectangle<float> glowBounds = ledBounds.withSizeKeepingCentre(
                    ledBounds.getWidth() * glowScale,
                    ledBounds.getHeight() * glowScale);
                g.setColour(glowColour.withAlpha(0.1f * (4 - i))); // fading glow
                g.fillEllipse(glowBounds);
            }
        }
        
        
        juce::Colour ledColour = button.getToggleState() ? juce::Colour(0xFFa00b0b) : juce::Colours::black;

        g.setColour(ledColour);
        g.fillEllipse(ledBounds);
    }



private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SupersawLookAndFeel)
};
