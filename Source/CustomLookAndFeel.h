#pragma once

#include <JuceHeader.h>

namespace GainStage
{
    namespace Colours
    {
        const juce::Colour background      { 0xff0d0d0d };
        const juce::Colour panelBackground { 0xff1a1a2e };
        const juce::Colour panelBorder     { 0xff2d2d44 };
        const juce::Colour accent          { 0xff6c63ff };
        const juce::Colour accentDark      { 0xff4a42d4 };
        const juce::Colour accentBright    { 0xff8b83ff };
        const juce::Colour textPrimary     { 0xffe0e0e0 };
        const juce::Colour textSecondary   { 0xff888888 };
        const juce::Colour meterGreen      { 0xff00d26a };
        const juce::Colour meterYellow     { 0xffffc107 };
        const juce::Colour meterRed        { 0xffff4757 };
        const juce::Colour warning         { 0xffff9f43 };
        const juce::Colour success         { 0xff26de81 };
        const juce::Colour deltaBlue       { 0xff00b4d8 };
    }

    class CustomLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        CustomLookAndFeel()
        {
            setColour(juce::ResizableWindow::backgroundColourId, Colours::background);
            setColour(juce::Slider::thumbColourId, Colours::accent);
            setColour(juce::Slider::trackColourId, Colours::panelBorder);
            setColour(juce::Slider::backgroundColourId, Colours::panelBackground);
            setColour(juce::TextButton::buttonColourId, Colours::panelBackground);
            setColour(juce::TextButton::textColourOnId, Colours::textPrimary);
            setColour(juce::TextButton::textColourOffId, Colours::textPrimary);
            setColour(juce::ComboBox::backgroundColourId, Colours::panelBackground);
            setColour(juce::ComboBox::textColourId, Colours::textPrimary);
            setColour(juce::ComboBox::outlineColourId, Colours::panelBorder);
            setColour(juce::PopupMenu::backgroundColourId, Colours::panelBackground);
            setColour(juce::PopupMenu::textColourId, Colours::textPrimary);
            setColour(juce::PopupMenu::highlightedBackgroundColourId, Colours::accent);
            setColour(juce::Label::textColourId, Colours::textPrimary);
            setColour(juce::ToggleButton::textColourId, Colours::textPrimary);
            setColour(juce::ToggleButton::tickColourId, Colours::accent);
        }

        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                              juce::Slider& slider) override
        {
            auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(4);
            auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
            auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
            auto lineW = juce::jmin(6.0f, radius * 0.2f);
            auto arcRadius = radius - lineW * 0.5f;

            // Background arc
            juce::Path backgroundArc;
            backgroundArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(),
                                        arcRadius, arcRadius, 0.0f,
                                        rotaryStartAngle, rotaryEndAngle, true);
            g.setColour(Colours::panelBorder);
            g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            // Value arc with gradient
            if (slider.isEnabled())
            {
                juce::Path valueArc;
                valueArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(),
                                       arcRadius, arcRadius, 0.0f,
                                       rotaryStartAngle, toAngle, true);

                juce::ColourGradient gradient(Colours::accentDark, bounds.getCentreX(), bounds.getY(),
                                              Colours::accentBright, bounds.getCentreX(), bounds.getBottom(), false);
                g.setGradientFill(gradient);
                g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }

            // Thumb dot
            auto thumbWidth = lineW * 1.5f;
            juce::Point<float> thumbPoint(bounds.getCentreX() + (arcRadius - lineW) * std::cos(toAngle - juce::MathConstants<float>::halfPi),
                                          bounds.getCentreY() + (arcRadius - lineW) * std::sin(toAngle - juce::MathConstants<float>::halfPi));
            g.setColour(Colours::textPrimary);
            g.fillEllipse(juce::Rectangle<float>(thumbWidth, thumbWidth).withCentre(thumbPoint));
        }

        void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPos, float minSliderPos, float maxSliderPos,
                              const juce::Slider::SliderStyle style, juce::Slider& slider) override
        {
            if (style == juce::Slider::LinearHorizontal)
            {
                auto trackWidth = height * 0.3f;
                auto trackY = y + (height - trackWidth) * 0.5f;

                // Background track
                g.setColour(Colours::panelBorder);
                g.fillRoundedRectangle(float(x), trackY, float(width), trackWidth, trackWidth * 0.5f);

                // Filled portion
                auto filledWidth = sliderPos - x;
                if (filledWidth > 0)
                {
                    juce::ColourGradient gradient(Colours::accentDark, float(x), trackY,
                                                  Colours::accent, sliderPos, trackY, false);
                    g.setGradientFill(gradient);
                    g.fillRoundedRectangle(float(x), trackY, filledWidth, trackWidth, trackWidth * 0.5f);
                }

                // Thumb
                auto thumbSize = height * 0.7f;
                g.setColour(Colours::textPrimary);
                g.fillEllipse(sliderPos - thumbSize * 0.5f, y + (height - thumbSize) * 0.5f, thumbSize, thumbSize);

                // Thumb inner
                g.setColour(Colours::accent);
                g.fillEllipse(sliderPos - thumbSize * 0.3f, y + (height - thumbSize * 0.6f) * 0.5f, thumbSize * 0.6f, thumbSize * 0.6f);
            }
            else
            {
                LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
            }
        }

        void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
        {
            auto bounds = button.getLocalBounds().toFloat().reduced(2);

            // Background
            auto bgColour = button.getToggleState() ? Colours::accent : Colours::panelBackground;
            if (shouldDrawButtonAsHighlighted)
                bgColour = bgColour.brighter(0.1f);

            g.setColour(bgColour);
            g.fillRoundedRectangle(bounds, 6.0f);

            // Border
            g.setColour(button.getToggleState() ? Colours::accentBright : Colours::panelBorder);
            g.drawRoundedRectangle(bounds, 6.0f, 1.5f);

            // Text
            g.setColour(button.getToggleState() ? Colours::textPrimary : Colours::textSecondary);
            g.setFont(juce::Font(13.0f, juce::Font::bold));
            g.drawText(button.getButtonText(), bounds, juce::Justification::centred);
        }

        void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                          int buttonX, int buttonY, int buttonW, int buttonH,
                          juce::ComboBox& box) override
        {
            auto bounds = box.getLocalBounds().toFloat().reduced(1);

            g.setColour(Colours::panelBackground);
            g.fillRoundedRectangle(bounds, 6.0f);

            g.setColour(Colours::panelBorder);
            g.drawRoundedRectangle(bounds, 6.0f, 1.5f);

            // Arrow
            auto arrowZone = juce::Rectangle<float>(buttonX, buttonY, buttonW, buttonH).reduced(8);
            juce::Path arrow;
            arrow.addTriangle(arrowZone.getX(), arrowZone.getCentreY() - 3,
                              arrowZone.getRight(), arrowZone.getCentreY() - 3,
                              arrowZone.getCentreX(), arrowZone.getCentreY() + 3);
            g.setColour(Colours::textSecondary);
            g.fillPath(arrow);
        }

        juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override
        {
            return juce::Font(juce::jmin(14.0f, buttonHeight * 0.6f), juce::Font::bold);
        }

        void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                                  bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
        {
            auto bounds = button.getLocalBounds().toFloat().reduced(1);

            auto baseColour = backgroundColour;
            if (shouldDrawButtonAsDown)
                baseColour = Colours::accent;
            else if (shouldDrawButtonAsHighlighted)
                baseColour = baseColour.brighter(0.1f);

            g.setColour(baseColour);
            g.fillRoundedRectangle(bounds, 6.0f);

            g.setColour(Colours::panelBorder);
            g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
        }
    };
}
