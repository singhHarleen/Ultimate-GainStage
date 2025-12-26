#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CustomLookAndFeel.h"

class GradientMeter : public juce::Component, public juce::Timer
{
public:
    enum class MeterType { Normal, GainReduction, Delta };

    GradientMeter(const juce::String& label, MeterType type = MeterType::Normal)
        : label_(label), type_(type)
    {
        startTimerHz(30);
    }

    void setLevel(float leveldB)
    {
        targetLevel_ = juce::jmap(juce::jlimit(-60.0f, 6.0f, leveldB), -60.0f, 6.0f, 0.0f, 1.0f);
    }

    void setGainReduction(float dB)
    {
        gainReductionDb_ = dB;
        targetLevel_ = juce::jmap(juce::jlimit(-20.0f, 20.0f, std::abs(dB)), 0.0f, 20.0f, 0.0f, 1.0f);
    }

    void timerCallback() override
    {
        float diff = targetLevel_ - currentLevel_;
        if (std::abs(diff) > 0.001f)
        {
            currentLevel_ += diff * (diff > 0 ? 0.5f : 0.2f);
            repaint();
        }
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        auto labelHeight = 22.0f;
        auto valueHeight = 18.0f;
        auto meterBounds = bounds.reduced(4).withTrimmedTop(labelHeight).withTrimmedBottom(valueHeight);

        // Label
        g.setColour(GainStage::Colours::textSecondary);
        g.setFont(juce::Font(11.0f, juce::Font::bold));
        g.drawText(label_, bounds.removeFromTop(labelHeight), juce::Justification::centred);

        // Meter background with rounded corners
        g.setColour(GainStage::Colours::panelBackground);
        g.fillRoundedRectangle(meterBounds, 4.0f);

        // Meter border
        g.setColour(GainStage::Colours::panelBorder);
        g.drawRoundedRectangle(meterBounds, 4.0f, 1.0f);

        // Meter fill
        auto fillBounds = meterBounds.reduced(2);
        auto meterHeight = fillBounds.getHeight() * currentLevel_;
        auto fillRect = fillBounds.removeFromBottom(static_cast<int>(meterHeight));

        if (meterHeight > 0)
        {
            juce::ColourGradient gradient;

            if (type_ == MeterType::GainReduction)
            {
                if (gainReductionDb_ > 0)
                    gradient = juce::ColourGradient(GainStage::Colours::meterGreen, fillRect.getX(), fillRect.getBottom(),
                                                    GainStage::Colours::meterYellow, fillRect.getX(), fillRect.getY(), false);
                else
                    gradient = juce::ColourGradient(GainStage::Colours::warning, fillRect.getX(), fillRect.getBottom(),
                                                    GainStage::Colours::meterRed, fillRect.getX(), fillRect.getY(), false);
            }
            else if (type_ == MeterType::Delta)
            {
                gradient = juce::ColourGradient(GainStage::Colours::deltaBlue.darker(0.3f), fillRect.getX(), fillRect.getBottom(),
                                                GainStage::Colours::deltaBlue.brighter(0.2f), fillRect.getX(), fillRect.getY(), false);
            }
            else
            {
                gradient = juce::ColourGradient(GainStage::Colours::meterGreen, fillRect.getX(), fillRect.getBottom(),
                                                GainStage::Colours::meterYellow, fillRect.getX(), fillRect.getY() + fillRect.getHeight() * 0.3f, false);
                gradient.addColour(0.85, GainStage::Colours::meterYellow);
                gradient.addColour(1.0, GainStage::Colours::meterRed);
            }

            g.setGradientFill(gradient);
            g.fillRoundedRectangle(fillRect, 2.0f);

            // Glow effect
            g.setColour(juce::Colours::white.withAlpha(0.1f));
            g.fillRoundedRectangle(fillRect.removeFromLeft(fillRect.getWidth() * 0.3f), 2.0f);
        }

        // dB markers
        g.setColour(GainStage::Colours::textSecondary.withAlpha(0.5f));
        g.setFont(8.0f);
        auto markerX = meterBounds.getRight() + 2;
        g.drawText("0", markerX, meterBounds.getY() + meterBounds.getHeight() * 0.09f - 4, 20, 10, juce::Justification::left);
        g.drawText("-20", markerX, meterBounds.getY() + meterBounds.getHeight() * 0.4f - 4, 20, 10, juce::Justification::left);
        g.drawText("-40", markerX, meterBounds.getY() + meterBounds.getHeight() * 0.7f - 4, 20, 10, juce::Justification::left);

        // Value display
        auto valueStr = (type_ == MeterType::GainReduction)
            ? juce::String(gainReductionDb_, 1) + " dB"
            : juce::String(juce::jmap(currentLevel_, 0.0f, 1.0f, -60.0f, 6.0f), 1) + " dB";

        g.setColour(GainStage::Colours::textPrimary);
        g.setFont(juce::Font(11.0f, juce::Font::bold));
        g.drawText(valueStr, bounds.removeFromBottom(valueHeight), juce::Justification::centred);
    }

private:
    juce::String label_;
    MeterType type_;
    float currentLevel_ = 0.0f;
    float targetLevel_ = 0.0f;
    float gainReductionDb_ = 0.0f;
};

class StatusIndicator : public juce::Component
{
public:
    void setStatus(bool active, const juce::String& text, juce::Colour colour)
    {
        active_ = active;
        text_ = text;
        colour_ = colour;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2);

        // Background
        g.setColour(active_ ? colour_.withAlpha(0.2f) : GainStage::Colours::panelBackground);
        g.fillRoundedRectangle(bounds, 6.0f);

        // Border
        g.setColour(active_ ? colour_ : GainStage::Colours::panelBorder);
        g.drawRoundedRectangle(bounds, 6.0f, 1.5f);

        // Indicator dot
        auto dotBounds = bounds.removeFromLeft(bounds.getHeight()).reduced(8);
        g.setColour(active_ ? colour_ : GainStage::Colours::textSecondary.withAlpha(0.3f));
        g.fillEllipse(dotBounds);

        if (active_)
        {
            // Glow
            g.setColour(colour_.withAlpha(0.3f));
            g.fillEllipse(dotBounds.expanded(2));
        }

        // Text
        g.setColour(active_ ? GainStage::Colours::textPrimary : GainStage::Colours::textSecondary);
        g.setFont(juce::Font(12.0f, juce::Font::bold));
        g.drawText(text_, bounds.reduced(4, 0), juce::Justification::centredLeft);
    }

private:
    bool active_ = false;
    juce::String text_;
    juce::Colour colour_ = GainStage::Colours::success;
};

class UltimateGainStageAudioProcessorEditor : public juce::AudioProcessorEditor,
                                               public juce::Timer
{
public:
    UltimateGainStageAudioProcessorEditor(UltimateGainStageAudioProcessor&);
    ~UltimateGainStageAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    void updateUIForMode();

    UltimateGainStageAudioProcessor& audioProcessor;
    GainStage::CustomLookAndFeel customLookAndFeel_;

    // Header
    juce::ToggleButton modeToggle_{ "AFTER" };
    juce::ComboBox pairIdCombo_;
    StatusIndicator pairStatus_;
    juce::ToggleButton bypassToggle_{ "BYPASS" };

    // Meters
    GradientMeter beforeMeter_{ "BEFORE" };
    GradientMeter afterMeter_{ "AFTER" };
    GradientMeter gainMeter_{ "GAIN", GradientMeter::MeterType::GainReduction };
    GradientMeter outputMeter_{ "OUTPUT" };
    GradientMeter deltaMeter_{ "DELTA", GradientMeter::MeterType::Delta };

    // Status indicators
    StatusIndicator compensatingStatus_;
    StatusIndicator warningStatus_;
    StatusIndicator clippingStatus_;

    // Controls
    juce::ComboBox measurementModeCombo_;
    juce::ComboBox rmsWindowCombo_;
    juce::Slider attackSlider_;
    juce::Slider releaseSlider_;
    juce::Slider toleranceSlider_;
    juce::Label attackLabel_{ {}, "Attack" };
    juce::Label releaseLabel_{ {}, "Release" };
    juce::Label toleranceLabel_{ {}, "Tolerance" };

    // Delta section
    juce::ToggleButton deltaEnableToggle_{ "DELTA" };
    juce::ToggleButton deltaSoloToggle_{ "SOLO" };
    juce::Slider deltaGainSlider_;
    juce::Label deltaGainLabel_{ {}, "Delta Gain" };

    // Listen controls
    juce::ToggleButton listenBeforeToggle_{ "LISTEN REF" };

    // Latency
    juce::Slider latencyOffsetSlider_;
    juce::Label latencyLabel_{ {}, "Latency Offset (samples)" };

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> pairIdAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> measurementModeAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> rmsWindowAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> toleranceAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> deltaEnableAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> deltaSoloAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> deltaGainAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> listenBeforeAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> latencyOffsetAttachment_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UltimateGainStageAudioProcessorEditor)
};
