#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class LevelMeter : public juce::Component, public juce::Timer
{
public:
    LevelMeter(const juce::String& label) : label_(label)
    {
        startTimerHz(30);
    }

    void setLevel(float leveldB)
    {
        targetLevel_ = juce::jmap(juce::jlimit(-60.0f, 0.0f, leveldB), -60.0f, 0.0f, 0.0f, 1.0f);
    }

    void timerCallback() override
    {
        float diff = targetLevel_ - currentLevel_;
        if (std::abs(diff) > 0.001f)
        {
            currentLevel_ += diff * 0.3f;
            repaint();
        }
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().reduced(2);
        auto labelArea = bounds.removeFromTop(20);

        g.setColour(juce::Colours::white);
        g.setFont(12.0f);
        g.drawText(label_, labelArea, juce::Justification::centred);

        g.setColour(juce::Colours::darkgrey);
        g.fillRect(bounds);

        auto meterHeight = static_cast<int>(bounds.getHeight() * currentLevel_);
        auto meterBounds = bounds.removeFromBottom(meterHeight);

        juce::Colour meterColour = currentLevel_ > 0.9f ? juce::Colours::red :
                                   currentLevel_ > 0.7f ? juce::Colours::yellow :
                                   juce::Colours::green;
        g.setColour(meterColour);
        g.fillRect(meterBounds);
    }

private:
    juce::String label_;
    float currentLevel_ = 0.0f;
    float targetLevel_ = 0.0f;
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

    juce::ToggleButton modeToggle_{ "AFTER" };
    juce::ComboBox pairIdCombo_;
    juce::Label pairStatusLabel_;

    LevelMeter beforeMeter_{ "BEFORE" };
    LevelMeter afterMeter_{ "AFTER" };
    LevelMeter outputMeter_{ "OUTPUT" };
    LevelMeter deltaMeter_{ "DELTA" };

    juce::Label gainReductionLabel_;
    juce::Label compensatingLabel_;

    juce::ComboBox measurementModeCombo_;
    juce::ComboBox rmsWindowCombo_;

    juce::Slider attackSlider_;
    juce::Slider releaseSlider_;
    juce::Slider toleranceSlider_;

    juce::ToggleButton deltaEnableToggle_{ "Delta" };
    juce::ToggleButton deltaSoloToggle_{ "Solo" };
    juce::Slider deltaGainSlider_;

    juce::ToggleButton listenBeforeToggle_{ "Listen Before" };
    juce::ToggleButton bypassToggle_{ "Bypass" };

    juce::Slider latencyOffsetSlider_;
    juce::Label latencyLabel_{ {}, "Latency Offset:" };

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment_;
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
