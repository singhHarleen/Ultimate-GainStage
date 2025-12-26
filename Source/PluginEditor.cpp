#include "PluginProcessor.h"
#include "PluginEditor.h"

UltimateGainStageAudioProcessorEditor::UltimateGainStageAudioProcessorEditor(UltimateGainStageAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&customLookAndFeel_);

    // Mode toggle
    modeToggle_.setButtonText("AFTER");
    modeToggle_.onClick = [this]()
    {
        auto* param = audioProcessor.getAPVTS().getParameter(GainStage::ParamIDs::MODE);
        param->setValueNotifyingHost(modeToggle_.getToggleState() ? 1.0f : 0.0f);
        updateUIForMode();
    };
    addAndMakeVisible(modeToggle_);

    // Pair ID
    for (int i = 1; i <= 16; ++i)
        pairIdCombo_.addItem("Pair " + juce::String(i), i);
    pairIdCombo_.setSelectedId(1);
    addAndMakeVisible(pairIdCombo_);
    pairIdAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getAPVTS(), GainStage::ParamIDs::PAIR_ID, pairIdCombo_);

    // Status indicators
    addAndMakeVisible(pairStatus_);
    addAndMakeVisible(compensatingStatus_);
    addAndMakeVisible(warningStatus_);
    addAndMakeVisible(clippingStatus_);

    // Bypass
    addAndMakeVisible(bypassToggle_);
    bypassAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), GainStage::ParamIDs::BYPASS, bypassToggle_);

    // Meters
    addAndMakeVisible(beforeMeter_);
    addAndMakeVisible(afterMeter_);
    addAndMakeVisible(gainMeter_);
    addAndMakeVisible(outputMeter_);
    addAndMakeVisible(deltaMeter_);

    // Measurement mode
    measurementModeCombo_.addItem("RMS", 1);
    measurementModeCombo_.addItem("Peak", 2);
    addAndMakeVisible(measurementModeCombo_);
    measurementModeAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getAPVTS(), GainStage::ParamIDs::MEASUREMENT_MODE, measurementModeCombo_);

    // RMS Window
    rmsWindowCombo_.addItem("50 ms", 1);
    rmsWindowCombo_.addItem("100 ms", 2);
    rmsWindowCombo_.addItem("300 ms", 3);
    addAndMakeVisible(rmsWindowCombo_);
    rmsWindowAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getAPVTS(), GainStage::ParamIDs::RMS_WINDOW, rmsWindowCombo_);

    // Attack slider
    attackSlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    attackSlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    attackSlider_.setTextValueSuffix(" ms");
    addAndMakeVisible(attackSlider_);
    attackAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), GainStage::ParamIDs::ATTACK_TIME, attackSlider_);
    attackLabel_.setJustificationType(juce::Justification::centred);
    attackLabel_.setColour(juce::Label::textColourId, GainStage::Colours::textSecondary);
    addAndMakeVisible(attackLabel_);

    // Release slider
    releaseSlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    releaseSlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    releaseSlider_.setTextValueSuffix(" ms");
    addAndMakeVisible(releaseSlider_);
    releaseAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), GainStage::ParamIDs::RELEASE_TIME, releaseSlider_);
    releaseLabel_.setJustificationType(juce::Justification::centred);
    releaseLabel_.setColour(juce::Label::textColourId, GainStage::Colours::textSecondary);
    addAndMakeVisible(releaseLabel_);

    // Tolerance slider
    toleranceSlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    toleranceSlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    toleranceSlider_.setTextValueSuffix(" dB");
    addAndMakeVisible(toleranceSlider_);
    toleranceAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), GainStage::ParamIDs::TOLERANCE, toleranceSlider_);
    toleranceLabel_.setJustificationType(juce::Justification::centred);
    toleranceLabel_.setColour(juce::Label::textColourId, GainStage::Colours::textSecondary);
    addAndMakeVisible(toleranceLabel_);

    // Delta controls
    addAndMakeVisible(deltaEnableToggle_);
    deltaEnableAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), GainStage::ParamIDs::DELTA_ENABLED, deltaEnableToggle_);

    addAndMakeVisible(deltaSoloToggle_);
    deltaSoloAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), GainStage::ParamIDs::DELTA_SOLO, deltaSoloToggle_);

    deltaGainSlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    deltaGainSlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    deltaGainSlider_.setTextValueSuffix(" dB");
    addAndMakeVisible(deltaGainSlider_);
    deltaGainAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), GainStage::ParamIDs::DELTA_GAIN, deltaGainSlider_);
    deltaGainLabel_.setJustificationType(juce::Justification::centred);
    deltaGainLabel_.setColour(juce::Label::textColourId, GainStage::Colours::textSecondary);
    addAndMakeVisible(deltaGainLabel_);

    // Listen before
    addAndMakeVisible(listenBeforeToggle_);
    listenBeforeAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), GainStage::ParamIDs::LISTEN_BEFORE, listenBeforeToggle_);

    // Latency offset
    latencyOffsetSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    latencyOffsetSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 70, 20);
    addAndMakeVisible(latencyOffsetSlider_);
    latencyOffsetAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), GainStage::ParamIDs::LATENCY_OFFSET, latencyOffsetSlider_);
    latencyLabel_.setColour(juce::Label::textColourId, GainStage::Colours::textSecondary);
    addAndMakeVisible(latencyLabel_);

    // Initialize mode from parameter
    auto* modeParam = audioProcessor.getAPVTS().getRawParameterValue(GainStage::ParamIDs::MODE);
    modeToggle_.setToggleState(modeParam->load() > 0.5f, juce::dontSendNotification);

    updateUIForMode();
    startTimerHz(30);
    setSize(650, 500);
}

UltimateGainStageAudioProcessorEditor::~UltimateGainStageAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void UltimateGainStageAudioProcessorEditor::updateUIForMode()
{
    bool isAfterMode = modeToggle_.getToggleState();
    modeToggle_.setButtonText(isAfterMode ? "AFTER" : "BEFORE");

    // Show/hide controls based on mode
    afterMeter_.setVisible(isAfterMode);
    gainMeter_.setVisible(isAfterMode);
    outputMeter_.setVisible(isAfterMode);
    deltaMeter_.setVisible(isAfterMode);

    compensatingStatus_.setVisible(isAfterMode);
    warningStatus_.setVisible(isAfterMode);
    clippingStatus_.setVisible(isAfterMode);

    measurementModeCombo_.setVisible(isAfterMode);
    rmsWindowCombo_.setVisible(isAfterMode);

    attackSlider_.setVisible(isAfterMode);
    releaseSlider_.setVisible(isAfterMode);
    toleranceSlider_.setVisible(isAfterMode);
    attackLabel_.setVisible(isAfterMode);
    releaseLabel_.setVisible(isAfterMode);
    toleranceLabel_.setVisible(isAfterMode);

    deltaEnableToggle_.setVisible(isAfterMode);
    deltaSoloToggle_.setVisible(isAfterMode);
    deltaGainSlider_.setVisible(isAfterMode);
    deltaGainLabel_.setVisible(isAfterMode);

    listenBeforeToggle_.setVisible(isAfterMode);
    latencyOffsetSlider_.setVisible(isAfterMode);
    latencyLabel_.setVisible(isAfterMode);

    if (isAfterMode)
        setSize(650, 500);
    else
        setSize(320, 280);

    resized();
}

void UltimateGainStageAudioProcessorEditor::timerCallback()
{
    bool isPaired = audioProcessor.isPaired();
    auto mode = audioProcessor.getInstanceMode();

    if (mode == GainStage::InstanceMode::Before)
    {
        pairStatus_.setStatus(true, "SENDING", GainStage::Colours::success);
        beforeMeter_.setLevel(audioProcessor.getBeforeLeveldB());
    }
    else
    {
        pairStatus_.setStatus(isPaired, isPaired ? "PAIRED" : "NOT PAIRED",
                              isPaired ? GainStage::Colours::success : GainStage::Colours::meterRed);

        beforeMeter_.setLevel(audioProcessor.getBeforeLeveldB());
        afterMeter_.setLevel(audioProcessor.getAfterLeveldB());
        gainMeter_.setGainReduction(audioProcessor.getGainReductionDB());
        outputMeter_.setLevel(audioProcessor.getOutputLeveldB());
        deltaMeter_.setLevel(audioProcessor.getDeltaLeveldB());

        compensatingStatus_.setStatus(audioProcessor.isCompensating(), "COMPENSATING", GainStage::Colours::accent);
        warningStatus_.setStatus(audioProcessor.isWarning(), "HIGH GAIN", GainStage::Colours::warning);
        clippingStatus_.setStatus(audioProcessor.isClipping(), "CLIPPING", GainStage::Colours::meterRed);
    }
}

void UltimateGainStageAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Background gradient
    juce::ColourGradient bgGradient(GainStage::Colours::background, 0, 0,
                                     GainStage::Colours::panelBackground.darker(0.3f), 0, getHeight(), false);
    g.setGradientFill(bgGradient);
    g.fillAll();

    auto bounds = getLocalBounds();

    // Header background
    auto headerBounds = bounds.removeFromTop(60);
    g.setColour(GainStage::Colours::panelBackground.withAlpha(0.5f));
    g.fillRect(headerBounds);

    // Header line
    g.setColour(GainStage::Colours::accent);
    g.fillRect(headerBounds.removeFromBottom(2));

    // Title
    g.setColour(GainStage::Colours::textPrimary);
    g.setFont(juce::Font(22.0f, juce::Font::bold));
    auto mode = audioProcessor.getInstanceMode();
    juce::String title = (mode == GainStage::InstanceMode::Before) ? "GAINSTAGE" : "GAINSTAGE PRO";
    g.drawText(title, 20, 15, 200, 30, juce::Justification::centredLeft);

    // Subtitle
    g.setColour(GainStage::Colours::textSecondary);
    g.setFont(juce::Font(11.0f));
    g.drawText(mode == GainStage::InstanceMode::Before ? "Reference Capture" : "Gain Compensation + Delta Monitor",
               20, 38, 300, 20, juce::Justification::centredLeft);

    // Section backgrounds for After mode
    if (mode == GainStage::InstanceMode::After)
    {
        auto contentBounds = getLocalBounds().reduced(10).withTrimmedTop(60);

        // Meters section
        auto metersSection = contentBounds.removeFromTop(200);
        g.setColour(GainStage::Colours::panelBackground.withAlpha(0.3f));
        g.fillRoundedRectangle(metersSection.toFloat(), 8.0f);

        contentBounds.removeFromTop(10);

        // Controls section
        auto controlsSection = contentBounds.removeFromTop(120);
        g.setColour(GainStage::Colours::panelBackground.withAlpha(0.3f));
        g.fillRoundedRectangle(controlsSection.toFloat(), 8.0f);

        contentBounds.removeFromTop(10);

        // Delta section
        auto deltaSection = contentBounds;
        g.setColour(GainStage::Colours::deltaBlue.withAlpha(0.1f));
        g.fillRoundedRectangle(deltaSection.toFloat(), 8.0f);
        g.setColour(GainStage::Colours::deltaBlue.withAlpha(0.3f));
        g.drawRoundedRectangle(deltaSection.toFloat(), 8.0f, 1.0f);
    }
}

void UltimateGainStageAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // Header
    auto headerBounds = bounds.removeFromTop(60).reduced(10, 10);

    auto headerLeft = headerBounds.removeFromLeft(300);
    headerLeft.removeFromLeft(180); // Skip title area

    modeToggle_.setBounds(headerLeft.removeFromLeft(80).reduced(2));
    headerLeft.removeFromLeft(10);
    pairIdCombo_.setBounds(headerLeft.removeFromLeft(90).reduced(2));

    auto headerRight = headerBounds.removeFromRight(200);
    bypassToggle_.setBounds(headerRight.removeFromRight(80).reduced(2));
    headerRight.removeFromRight(10);
    pairStatus_.setBounds(headerRight);

    bounds.reduce(10, 0);

    bool isAfterMode = modeToggle_.getToggleState();

    if (!isAfterMode)
    {
        // Before mode - simple layout
        auto meterArea = bounds.reduced(40, 20);
        beforeMeter_.setBounds(meterArea.withSizeKeepingCentre(80, meterArea.getHeight()));
    }
    else
    {
        // After mode - full layout
        auto metersSection = bounds.removeFromTop(200).reduced(10);

        // Status indicators row
        auto statusRow = metersSection.removeFromTop(28);
        int statusWidth = (statusRow.getWidth() - 20) / 3;
        compensatingStatus_.setBounds(statusRow.removeFromLeft(statusWidth));
        statusRow.removeFromLeft(10);
        warningStatus_.setBounds(statusRow.removeFromLeft(statusWidth));
        statusRow.removeFromLeft(10);
        clippingStatus_.setBounds(statusRow);

        metersSection.removeFromTop(10);

        // Meters
        int meterWidth = 70;
        int numMeters = 5;
        int totalMeterWidth = meterWidth * numMeters;
        int spacing = (metersSection.getWidth() - totalMeterWidth) / (numMeters + 1);

        int x = spacing;
        beforeMeter_.setBounds(x, metersSection.getY(), meterWidth, metersSection.getHeight());
        x += meterWidth + spacing;
        afterMeter_.setBounds(x, metersSection.getY(), meterWidth, metersSection.getHeight());
        x += meterWidth + spacing;
        gainMeter_.setBounds(x, metersSection.getY(), meterWidth, metersSection.getHeight());
        x += meterWidth + spacing;
        outputMeter_.setBounds(x, metersSection.getY(), meterWidth, metersSection.getHeight());
        x += meterWidth + spacing;
        deltaMeter_.setBounds(x, metersSection.getY(), meterWidth, metersSection.getHeight());

        bounds.removeFromTop(10);

        // Controls section
        auto controlsSection = bounds.removeFromTop(120).reduced(10);

        auto topControls = controlsSection.removeFromTop(30);
        measurementModeCombo_.setBounds(topControls.removeFromLeft(100));
        topControls.removeFromLeft(10);
        rmsWindowCombo_.setBounds(topControls.removeFromLeft(100));

        controlsSection.removeFromTop(10);

        // Knobs row
        auto knobsRow = controlsSection;
        int knobSize = 70;
        int knobSpacing = (knobsRow.getWidth() - knobSize * 3) / 4;

        auto attackBounds = knobsRow.removeFromLeft(knobSpacing + knobSize).removeFromRight(knobSize);
        attackLabel_.setBounds(attackBounds.removeFromTop(16));
        attackSlider_.setBounds(attackBounds);

        auto releaseBounds = knobsRow.removeFromLeft(knobSpacing + knobSize).removeFromRight(knobSize);
        releaseLabel_.setBounds(releaseBounds.removeFromTop(16));
        releaseSlider_.setBounds(releaseBounds);

        auto toleranceBounds = knobsRow.removeFromLeft(knobSpacing + knobSize).removeFromRight(knobSize);
        toleranceLabel_.setBounds(toleranceBounds.removeFromTop(16));
        toleranceSlider_.setBounds(toleranceBounds);

        bounds.removeFromTop(10);

        // Delta section
        auto deltaSection = bounds.reduced(10);

        auto deltaTopRow = deltaSection.removeFromTop(30);
        deltaEnableToggle_.setBounds(deltaTopRow.removeFromLeft(80));
        deltaTopRow.removeFromLeft(10);
        deltaSoloToggle_.setBounds(deltaTopRow.removeFromLeft(70));
        deltaTopRow.removeFromLeft(20);
        listenBeforeToggle_.setBounds(deltaTopRow.removeFromLeft(100));

        deltaSection.removeFromTop(5);

        auto deltaBottomRow = deltaSection.removeFromTop(70);
        auto deltaKnobArea = deltaBottomRow.removeFromLeft(90);
        deltaGainLabel_.setBounds(deltaKnobArea.removeFromTop(16));
        deltaGainSlider_.setBounds(deltaKnobArea);

        deltaBottomRow.removeFromLeft(20);
        auto latencyArea = deltaBottomRow;
        latencyLabel_.setBounds(latencyArea.removeFromTop(20));
        latencyOffsetSlider_.setBounds(latencyArea.removeFromTop(25));
    }
}
