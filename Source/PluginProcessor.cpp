#include "PluginProcessor.h"
#include "PluginEditor.h"

UltimateGainStageAudioProcessor::UltimateGainStageAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
#else
    :
#endif
    apvts_(*this, nullptr, "Parameters", GainStage::createParameterLayout())
{
    modeParam_ = dynamic_cast<juce::AudioParameterChoice*>(apvts_.getParameter(GainStage::ParamIDs::MODE));
    pairIdParam_ = dynamic_cast<juce::AudioParameterInt*>(apvts_.getParameter(GainStage::ParamIDs::PAIR_ID));
    inputGainParam_ = dynamic_cast<juce::AudioParameterFloat*>(apvts_.getParameter(GainStage::ParamIDs::INPUT_GAIN));
    outputGainParam_ = dynamic_cast<juce::AudioParameterFloat*>(apvts_.getParameter(GainStage::ParamIDs::OUTPUT_GAIN));
    bypassParam_ = dynamic_cast<juce::AudioParameterBool*>(apvts_.getParameter(GainStage::ParamIDs::BYPASS));
    measurementModeParam_ = dynamic_cast<juce::AudioParameterChoice*>(apvts_.getParameter(GainStage::ParamIDs::MEASUREMENT_MODE));
    rmsWindowParam_ = dynamic_cast<juce::AudioParameterChoice*>(apvts_.getParameter(GainStage::ParamIDs::RMS_WINDOW));
    attackTimeParam_ = dynamic_cast<juce::AudioParameterFloat*>(apvts_.getParameter(GainStage::ParamIDs::ATTACK_TIME));
    releaseTimeParam_ = dynamic_cast<juce::AudioParameterFloat*>(apvts_.getParameter(GainStage::ParamIDs::RELEASE_TIME));
    toleranceParam_ = dynamic_cast<juce::AudioParameterFloat*>(apvts_.getParameter(GainStage::ParamIDs::TOLERANCE));
    deltaEnabledParam_ = dynamic_cast<juce::AudioParameterBool*>(apvts_.getParameter(GainStage::ParamIDs::DELTA_ENABLED));
    deltaGainParam_ = dynamic_cast<juce::AudioParameterFloat*>(apvts_.getParameter(GainStage::ParamIDs::DELTA_GAIN));
    deltaSoloParam_ = dynamic_cast<juce::AudioParameterBool*>(apvts_.getParameter(GainStage::ParamIDs::DELTA_SOLO));
    listenBeforeParam_ = dynamic_cast<juce::AudioParameterBool*>(apvts_.getParameter(GainStage::ParamIDs::LISTEN_BEFORE));
    listenAfterParam_ = dynamic_cast<juce::AudioParameterBool*>(apvts_.getParameter(GainStage::ParamIDs::LISTEN_AFTER));
    latencyOffsetParam_ = dynamic_cast<juce::AudioParameterInt*>(apvts_.getParameter(GainStage::ParamIDs::LATENCY_OFFSET));
}

UltimateGainStageAudioProcessor::~UltimateGainStageAudioProcessor()
{
    if (getInstanceMode() == GainStage::InstanceMode::Before)
    {
        GainStage::SharedBufferManager::getInstance().setBeforeInstanceInactive(getPairID());
    }
}

const juce::String UltimateGainStageAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool UltimateGainStageAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool UltimateGainStageAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool UltimateGainStageAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double UltimateGainStageAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int UltimateGainStageAudioProcessor::getNumPrograms()
{
    return 1;
}

int UltimateGainStageAudioProcessor::getCurrentProgram()
{
    return 0;
}

void UltimateGainStageAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String UltimateGainStageAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void UltimateGainStageAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void UltimateGainStageAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate_ = sampleRate;
    currentBlockSize_ = samplesPerBlock;

    beforeAnalyzer_.prepare(sampleRate, samplesPerBlock);
    afterAnalyzer_.prepare(sampleRate, samplesPerBlock);
    deltaAnalyzer_.prepare(sampleRate, samplesPerBlock);
    outputAnalyzer_.prepare(sampleRate, samplesPerBlock);
    gainSmoother_.prepare(sampleRate);

    referenceBuffer_.setSize(getTotalNumInputChannels(), samplesPerBlock);
    referenceBuffer_.clear();

    int pairID = getPairID();
    GainStage::SharedBufferManager::getInstance().prepareBuffer(pairID, sampleRate, getTotalNumInputChannels());

    auto rmsWindow = static_cast<GainStage::RMSWindow>(rmsWindowParam_.load()->getIndex());
    int windowSamples = GainStage::rmsWindowToSamples(rmsWindow, sampleRate);
    beforeAnalyzer_.setRMSWindowSamples(windowSamples);
    afterAnalyzer_.setRMSWindowSamples(windowSamples);

    gainSmoother_.setAttackTime(attackTimeParam_.load()->get());
    gainSmoother_.setReleaseTime(releaseTimeParam_.load()->get());
}

void UltimateGainStageAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool UltimateGainStageAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

GainStage::InstanceMode UltimateGainStageAudioProcessor::getInstanceMode() const
{
    if (auto* param = modeParam_.load())
        return static_cast<GainStage::InstanceMode>(param->getIndex());
    return GainStage::InstanceMode::Before;
}

int UltimateGainStageAudioProcessor::getPairID() const
{
    if (auto* param = pairIdParam_.load())
        return param->get();
    return 1;
}

bool UltimateGainStageAudioProcessor::isPaired() const
{
    int pairID = getPairID();
    auto mode = getInstanceMode();

    if (mode == GainStage::InstanceMode::Before)
        return true;

    return GainStage::SharedBufferManager::getInstance().isBeforeInstanceActive(pairID);
}

void UltimateGainStageAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    if (bypassParam_.load()->get())
        return;

    float inputGaindB = inputGainParam_.load()->get();
    if (std::abs(inputGaindB) > 0.001f)
    {
        float inputGainLinear = juce::Decibels::decibelsToGain(inputGaindB);
        buffer.applyGain(inputGainLinear);
    }

    auto mode = getInstanceMode();
    if (mode == GainStage::InstanceMode::Before)
    {
        processBeforeMode(buffer);
    }
    else
    {
        processAfterMode(buffer);
    }
}

void UltimateGainStageAudioProcessor::processBeforeMode(juce::AudioBuffer<float>& buffer)
{
    int pairID = getPairID();

    GainStage::SharedBufferManager::getInstance().writeSamples(pairID, buffer, buffer.getNumSamples());

    beforeAnalyzer_.process(buffer);
    beforeLeveldB_.store(beforeAnalyzer_.getRMSdB());
}

void UltimateGainStageAudioProcessor::processAfterMode(juce::AudioBuffer<float>& buffer)
{
    int pairID = getPairID();
    int numSamples = buffer.getNumSamples();
    int latencyOffset = latencyOffsetParam_.load()->get();

    auto rmsWindow = static_cast<GainStage::RMSWindow>(rmsWindowParam_.load()->getIndex());
    int windowSamples = GainStage::rmsWindowToSamples(rmsWindow, currentSampleRate_);
    beforeAnalyzer_.setRMSWindowSamples(windowSamples);
    afterAnalyzer_.setRMSWindowSamples(windowSamples);

    gainSmoother_.setAttackTime(attackTimeParam_.load()->get());
    gainSmoother_.setReleaseTime(releaseTimeParam_.load()->get());

    if (referenceBuffer_.getNumSamples() < numSamples)
        referenceBuffer_.setSize(buffer.getNumChannels(), numSamples, false, true, true);

    GainStage::SharedBufferManager::getInstance().readSamples(pairID, referenceBuffer_, numSamples, latencyOffset);

    beforeAnalyzer_.process(referenceBuffer_);
    afterAnalyzer_.process(buffer);

    auto measurementMode = static_cast<GainStage::MeasurementMode>(measurementModeParam_.load()->getIndex());
    float beforeLevel, afterLevel;

    if (measurementMode == GainStage::MeasurementMode::RMS)
    {
        beforeLevel = beforeAnalyzer_.getRMSdB();
        afterLevel = afterAnalyzer_.getRMSdB();
    }
    else
    {
        beforeLevel = beforeAnalyzer_.getPeakdB();
        afterLevel = afterAnalyzer_.getPeakdB();
    }

    beforeLeveldB_.store(beforeLevel);
    afterLeveldB_.store(afterLevel);

    float tolerance = toleranceParam_.load()->get();
    float gainDifference = beforeLevel - afterLevel;

    bool shouldCompensate = std::abs(gainDifference) > tolerance && isPaired();
    isCompensating_.store(shouldCompensate);

    float targetGaindB = shouldCompensate ? gainDifference : 0.0f;
    targetGaindB = juce::jlimit(-40.0f, 40.0f, targetGaindB);

    float smoothedGaindB = gainSmoother_.process(targetGaindB);
    gainReductiondB_.store(smoothedGaindB);

    bool listenBefore = listenBeforeParam_.load()->get();
    bool deltaEnabled = deltaEnabledParam_.load()->get();
    bool deltaSolo = deltaSoloParam_.load()->get();

    if (listenBefore)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            buffer.copyFrom(ch, 0, referenceBuffer_, ch, 0, numSamples);
        }
    }
    else if (deltaEnabled || deltaSolo)
    {
        float deltaGaindB = deltaGainParam_.load()->get();
        float deltaGainLinear = juce::Decibels::decibelsToGain(deltaGaindB);

        juce::AudioBuffer<float> deltaBuffer(buffer.getNumChannels(), numSamples);

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            float* deltaData = deltaBuffer.getWritePointer(ch);
            const float* afterData = buffer.getReadPointer(ch);
            const float* beforeData = referenceBuffer_.getReadPointer(ch);

            for (int i = 0; i < numSamples; ++i)
            {
                deltaData[i] = (afterData[i] - beforeData[i]) * deltaGainLinear;
            }
        }

        deltaAnalyzer_.process(deltaBuffer);
        deltaLeveldB_.store(deltaAnalyzer_.getRMSdB());

        if (deltaSolo)
        {
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                buffer.copyFrom(ch, 0, deltaBuffer, ch, 0, numSamples);
            }
        }
        else
        {
            float compensationGain = juce::Decibels::decibelsToGain(smoothedGaindB);
            buffer.applyGain(compensationGain);
        }
    }
    else
    {
        float compensationGain = juce::Decibels::decibelsToGain(smoothedGaindB);
        buffer.applyGain(compensationGain);
    }

    float outputGaindB = outputGainParam_.load()->get();
    if (std::abs(outputGaindB) > 0.001f)
    {
        float outputGainLinear = juce::Decibels::decibelsToGain(outputGaindB);
        buffer.applyGain(outputGainLinear);
    }

    outputAnalyzer_.process(buffer);
    outputLeveldB_.store(outputAnalyzer_.getRMSdB());
}

bool UltimateGainStageAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* UltimateGainStageAudioProcessor::createEditor()
{
    return new UltimateGainStageAudioProcessorEditor(*this);
}

void UltimateGainStageAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts_.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void UltimateGainStageAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(apvts_.state.getType()))
        {
            apvts_.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new UltimateGainStageAudioProcessor();
}
