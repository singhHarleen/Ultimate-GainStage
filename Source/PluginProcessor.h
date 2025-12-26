#pragma once

#include <JuceHeader.h>
#include "SharedBuffer.h"
#include "Parameters.h"
#include "GainAnalyzer.h"

class UltimateGainStageAudioProcessor : public juce::AudioProcessor
{
public:
    UltimateGainStageAudioProcessor();
    ~UltimateGainStageAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts_; }

    GainStage::InstanceMode getInstanceMode() const;
    int getPairID() const;
    bool isPaired() const;

    float getBeforeLeveldB() const { return beforeLeveldB_.load(); }
    float getAfterLeveldB() const { return afterLeveldB_.load(); }
    float getGainReductionDB() const { return gainReductiondB_.load(); }
    float getDeltaLeveldB() const { return deltaLeveldB_.load(); }
    float getOutputLeveldB() const { return outputLeveldB_.load(); }
    bool isCompensating() const { return isCompensating_.load(); }
    bool isClipping() const { return isClipping_.load(); }
    bool isWarning() const { return std::abs(gainReductiondB_.load()) > 10.0f; }

private:
    void processBeforeMode(juce::AudioBuffer<float>& buffer);
    void processAfterMode(juce::AudioBuffer<float>& buffer);

    juce::AudioProcessorValueTreeState apvts_;

    std::atomic<juce::AudioParameterChoice*> modeParam_{ nullptr };
    std::atomic<juce::AudioParameterInt*> pairIdParam_{ nullptr };
    std::atomic<juce::AudioParameterFloat*> inputGainParam_{ nullptr };
    std::atomic<juce::AudioParameterFloat*> outputGainParam_{ nullptr };
    std::atomic<juce::AudioParameterBool*> bypassParam_{ nullptr };
    std::atomic<juce::AudioParameterChoice*> measurementModeParam_{ nullptr };
    std::atomic<juce::AudioParameterChoice*> rmsWindowParam_{ nullptr };
    std::atomic<juce::AudioParameterFloat*> attackTimeParam_{ nullptr };
    std::atomic<juce::AudioParameterFloat*> releaseTimeParam_{ nullptr };
    std::atomic<juce::AudioParameterFloat*> toleranceParam_{ nullptr };
    std::atomic<juce::AudioParameterBool*> deltaEnabledParam_{ nullptr };
    std::atomic<juce::AudioParameterFloat*> deltaGainParam_{ nullptr };
    std::atomic<juce::AudioParameterBool*> deltaSoloParam_{ nullptr };
    std::atomic<juce::AudioParameterBool*> listenBeforeParam_{ nullptr };
    std::atomic<juce::AudioParameterBool*> listenAfterParam_{ nullptr };
    std::atomic<juce::AudioParameterInt*> latencyOffsetParam_{ nullptr };

    double currentSampleRate_ = 48000.0;
    int currentBlockSize_ = 512;

    GainStage::GainAnalyzer beforeAnalyzer_;
    GainStage::GainAnalyzer afterAnalyzer_;
    GainStage::GainAnalyzer deltaAnalyzer_;
    GainStage::GainAnalyzer outputAnalyzer_;
    GainStage::GainSmoother gainSmoother_;

    juce::AudioBuffer<float> referenceBuffer_;

    std::atomic<float> beforeLeveldB_{ -100.0f };
    std::atomic<float> afterLeveldB_{ -100.0f };
    std::atomic<float> gainReductiondB_{ 0.0f };
    std::atomic<float> deltaLeveldB_{ -100.0f };
    std::atomic<float> outputLeveldB_{ -100.0f };
    std::atomic<bool> isCompensating_{ false };
    std::atomic<bool> isClipping_{ false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UltimateGainStageAudioProcessor)
};
