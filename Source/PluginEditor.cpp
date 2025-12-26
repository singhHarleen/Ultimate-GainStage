#include "PluginProcessor.h"
#include "PluginEditor.h"

UltimateGainStageAudioProcessorEditor::UltimateGainStageAudioProcessorEditor(UltimateGainStageAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    modeToggle_.setButtonText("AFTER");
    modeToggle_.onClick = [this]()
    {
        auto* param = audioProcessor.getAPVTS().getParameter(GainStage::ParamIDs::MODE);
        param->setValueNotifyingHost(modeToggle_.getToggleState() ? 1.0f : 0.0f);
        updateUIForMode();
    };
    addAndMakeVisible(modeToggle_);

    for (int i = 1; i <= 16; ++i)
        pairIdCombo_.addItem(juce::String(i), i);
    pairIdCombo_.setSelectedId(1);
    addAndMakeVisible(pairIdCombo_);
    pairIdAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getAPVTS(), GainStage::ParamIDs::PAIR_ID, pairIdCombo_);

    pairStatusLabel_.setJustificationType(juce::Justification::centred);
    pairStatusLabel_.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(pairStatusLabel_);

    addAndMakeVisible(beforeMeter_);
    addAndMakeVisible(afterMeter_);
    addAndMakeVisible(outputMeter_);
    addAndMakeVisible(deltaMeter_);

    gainReductionLabel_.setJustificationType(juce::Justification::centred);
    gainReductionLabel_.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(gainReductionLabel_);

    compensatingLabel_.setJustificationType(juce::Justification::centred);
    compensatingLabel_.setColour(juce::Label::textColourId, juce::Colours::orange);
    addAndMakeVisible(compensatingLabel_);

    measurementModeCombo_.addItem("RMS", 1);
    measurementModeCombo_.addItem("Peak", 2);
    addAndMakeVisible(measurementModeCombo_);
    measurementModeAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getAPVTS(), GainStage::ParamIDs::MEASUREMENT_MODE, measurementModeCombo_);

    rmsWindowCombo_.addItem("50ms", 1);
    rmsWindowCombo_.addItem("100ms", 2);
    rmsWindowCombo_.addItem("300ms", 3);
    addAndMakeVisible(rmsWindowCombo_);
    rmsWindowAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getAPVTS(), GainStage::ParamIDs::RMS_WINDOW, rmsWindowCombo_);

    attackSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    attackSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    attackSlider_.setTextValueSuffix(" ms");
    addAndMakeVisible(attackSlider_);
    attackAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), GainStage::ParamIDs::ATTACK_TIME, attackSlider_);

    releaseSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    releaseSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    releaseSlider_.setTextValueSuffix(" ms");
    addAndMakeVisible(releaseSlider_);
    releaseAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), GainStage::ParamIDs::RELEASE_TIME, releaseSlider_);

    toleranceSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    toleranceSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    toleranceSlider_.setTextValueSuffix(" dB");
    addAndMakeVisible(toleranceSlider_);
    toleranceAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), GainStage::ParamIDs::TOLERANCE, toleranceSlider_);

    addAndMakeVisible(deltaEnableToggle_);
    deltaEnableAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), GainStage::ParamIDs::DELTA_ENABLED, deltaEnableToggle_);

    addAndMakeVisible(deltaSoloToggle_);
    deltaSoloAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), GainStage::ParamIDs::DELTA_SOLO, deltaSoloToggle_);

    deltaGainSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    deltaGainSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    deltaGainSlider_.setTextValueSuffix(" dB");
    addAndMakeVisible(deltaGainSlider_);
    deltaGainAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), GainStage::ParamIDs::DELTA_GAIN, deltaGainSlider_);

    addAndMakeVisible(listenBeforeToggle_);
    listenBeforeAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), GainStage::ParamIDs::LISTEN_BEFORE, listenBeforeToggle_);

    addAndMakeVisible(bypassToggle_);
    bypassAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), GainStage::ParamIDs::BYPASS, bypassToggle_);

    latencyLabel_.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(latencyLabel_);

    latencyOffsetSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    latencyOffsetSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 70, 20);
    latencyOffsetSlider_.setTextValueSuffix(" smp");
    addAndMakeVisible(latencyOffsetSlider_);
    latencyOffsetAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), GainStage::ParamIDs::LATENCY_OFFSET, latencyOffsetSlider_);

    auto* modeParam = audioProcessor.getAPVTS().getRawParameterValue(GainStage::ParamIDs::MODE);
    modeToggle_.setToggleState(modeParam->load() > 0.5f, juce::dontSendNotification);

    updateUIForMode();

    startTimerHz(30);

    setSize(600, 450);
}

UltimateGainStageAudioProcessorEditor::~UltimateGainStageAudioProcessorEditor()
{
    stopTimer();
}

void UltimateGainStageAudioProcessorEditor::updateUIForMode()
{
    bool isAfterMode = modeToggle_.getToggleState();

    modeToggle_.setButtonText(isAfterMode ? "AFTER" : "BEFORE");

    afterMeter_.setVisible(isAfterMode);
    outputMeter_.setVisible(isAfterMode);
    deltaMeter_.setVisible(isAfterMode);
    gainReductionLabel_.setVisible(isAfterMode);
    compensatingLabel_.setVisible(isAfterMode);
    measurementModeCombo_.setVisible(isAfterMode);
    rmsWindowCombo_.setVisible(isAfterMode);
    attackSlider_.setVisible(isAfterMode);
    releaseSlider_.setVisible(isAfterMode);
    toleranceSlider_.setVisible(isAfterMode);
    deltaEnableToggle_.setVisible(isAfterMode);
    deltaSoloToggle_.setVisible(isAfterMode);
    deltaGainSlider_.setVisible(isAfterMode);
    listenBeforeToggle_.setVisible(isAfterMode);
    latencyLabel_.setVisible(isAfterMode);
    latencyOffsetSlider_.setVisible(isAfterMode);

    if (isAfterMode)
    {
        setSize(600, 450);
    }
    else
    {
        setSize(300, 200);
    }

    resized();
}

void UltimateGainStageAudioProcessorEditor::timerCallback()
{
    bool isPaired = audioProcessor.isPaired();
    auto mode = audioProcessor.getInstanceMode();

    if (mode == GainStage::InstanceMode::Before)
    {
        pairStatusLabel_.setText("BEFORE - Sending", juce::dontSendNotification);
        pairStatusLabel_.setColour(juce::Label::backgroundColourId, juce::Colours::darkgreen);
        beforeMeter_.setLevel(audioProcessor.getBeforeLeveldB());
    }
    else
    {
        if (isPaired)
        {
            pairStatusLabel_.setText("PAIRED", juce::dontSendNotification);
            pairStatusLabel_.setColour(juce::Label::backgroundColourId, juce::Colours::darkgreen);
        }
        else
        {
            pairStatusLabel_.setText("NOT PAIRED", juce::dontSendNotification);
            pairStatusLabel_.setColour(juce::Label::backgroundColourId, juce::Colours::darkred);
        }

        beforeMeter_.setLevel(audioProcessor.getBeforeLeveldB());
        afterMeter_.setLevel(audioProcessor.getAfterLeveldB());
        outputMeter_.setLevel(audioProcessor.getOutputLeveldB());
        deltaMeter_.setLevel(audioProcessor.getDeltaLeveldB());

        float grDb = audioProcessor.getGainReductionDB();
        gainReductionLabel_.setText(juce::String(grDb, 1) + " dB", juce::dontSendNotification);

        if (audioProcessor.isCompensating())
        {
            compensatingLabel_.setText("COMPENSATING", juce::dontSendNotification);
            compensatingLabel_.setColour(juce::Label::textColourId, juce::Colours::orange);
        }
        else
        {
            compensatingLabel_.setText("IDLE", juce::dontSendNotification);
            compensatingLabel_.setColour(juce::Label::textColourId, juce::Colours::grey);
        }
    }
}

void UltimateGainStageAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a2e));

    g.setColour(juce::Colours::white);
    g.setFont(18.0f);

    auto mode = audioProcessor.getInstanceMode();
    juce::String title = (mode == GainStage::InstanceMode::Before) ?
        "GAIN STAGE - BEFORE" : "GAIN STAGE PRO - AFTER";
    g.drawText(title, 10, 5, getWidth() - 20, 30, juce::Justification::centred);
}

void UltimateGainStageAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    bounds.removeFromTop(35);

    auto topRow = bounds.removeFromTop(30);
    modeToggle_.setBounds(topRow.removeFromLeft(100));
    topRow.removeFromLeft(10);

    auto pairArea = topRow.removeFromLeft(80);
    pairIdCombo_.setBounds(pairArea);
    topRow.removeFromLeft(10);

    pairStatusLabel_.setBounds(topRow.removeFromLeft(150));

    bypassToggle_.setBounds(topRow.removeFromRight(80));

    bounds.removeFromTop(10);

    bool isAfterMode = modeToggle_.getToggleState();

    if (!isAfterMode)
    {
        auto meterArea = bounds.reduced(20);
        beforeMeter_.setBounds(meterArea.withSizeKeepingCentre(60, meterArea.getHeight()));
    }
    else
    {
        auto metersRow = bounds.removeFromTop(150);

        int meterWidth = 60;
        int meterSpacing = 20;
        int totalWidth = meterWidth * 4 + meterSpacing * 3;
        int startX = (metersRow.getWidth() - totalWidth) / 2;

        beforeMeter_.setBounds(startX, metersRow.getY(), meterWidth, metersRow.getHeight());
        afterMeter_.setBounds(startX + meterWidth + meterSpacing, metersRow.getY(), meterWidth, metersRow.getHeight());
        outputMeter_.setBounds(startX + (meterWidth + meterSpacing) * 2, metersRow.getY(), meterWidth, metersRow.getHeight());
        deltaMeter_.setBounds(startX + (meterWidth + meterSpacing) * 3, metersRow.getY(), meterWidth, metersRow.getHeight());

        bounds.removeFromTop(10);

        auto statusRow = bounds.removeFromTop(25);
        gainReductionLabel_.setBounds(statusRow.removeFromLeft(statusRow.getWidth() / 2));
        compensatingLabel_.setBounds(statusRow);

        bounds.removeFromTop(15);

        auto controlsLeft = bounds.removeFromLeft(bounds.getWidth() / 2).reduced(5);
        auto controlsRight = bounds.reduced(5);

        auto modeRow = controlsLeft.removeFromTop(25);
        measurementModeCombo_.setBounds(modeRow.removeFromLeft(80));
        modeRow.removeFromLeft(10);
        rmsWindowCombo_.setBounds(modeRow.removeFromLeft(80));

        controlsLeft.removeFromTop(10);

        attackSlider_.setBounds(controlsLeft.removeFromTop(25));
        controlsLeft.removeFromTop(5);
        releaseSlider_.setBounds(controlsLeft.removeFromTop(25));
        controlsLeft.removeFromTop(5);
        toleranceSlider_.setBounds(controlsLeft.removeFromTop(25));

        controlsLeft.removeFromTop(10);
        auto latencyRow = controlsLeft.removeFromTop(25);
        latencyLabel_.setBounds(latencyRow.removeFromLeft(100));
        latencyOffsetSlider_.setBounds(latencyRow);

        auto deltaRow = controlsRight.removeFromTop(25);
        deltaEnableToggle_.setBounds(deltaRow.removeFromLeft(70));
        deltaSoloToggle_.setBounds(deltaRow.removeFromLeft(60));

        controlsRight.removeFromTop(5);
        deltaGainSlider_.setBounds(controlsRight.removeFromTop(25));

        controlsRight.removeFromTop(20);
        listenBeforeToggle_.setBounds(controlsRight.removeFromTop(25));
    }
}
