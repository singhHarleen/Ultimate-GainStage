#pragma once

#include <JuceHeader.h>

namespace GainStage
{
    namespace ParamIDs
    {
        inline constexpr const char* MODE = "mode";
        inline constexpr const char* PAIR_ID = "pairId";
        inline constexpr const char* INPUT_GAIN = "inputGain";
        inline constexpr const char* OUTPUT_GAIN = "outputGain";
        inline constexpr const char* BYPASS = "bypass";

        inline constexpr const char* MEASUREMENT_MODE = "measurementMode";
        inline constexpr const char* RMS_WINDOW = "rmsWindow";
        inline constexpr const char* ATTACK_TIME = "attackTime";
        inline constexpr const char* RELEASE_TIME = "releaseTime";
        inline constexpr const char* TOLERANCE = "tolerance";

        inline constexpr const char* DELTA_ENABLED = "deltaEnabled";
        inline constexpr const char* DELTA_GAIN = "deltaGain";
        inline constexpr const char* DELTA_SOLO = "deltaSolo";

        inline constexpr const char* LISTEN_BEFORE = "listenBefore";
        inline constexpr const char* LISTEN_AFTER = "listenAfter";

        inline constexpr const char* LATENCY_OFFSET = "latencyOffset";
    }

    namespace ParamDefaults
    {
        constexpr float INPUT_GAIN = 0.0f;
        constexpr float OUTPUT_GAIN = 0.0f;
        constexpr int PAIR_ID = 1;
        constexpr int MODE = 0;
        constexpr bool BYPASS = false;

        constexpr int MEASUREMENT_MODE = 0;
        constexpr int RMS_WINDOW = 1;
        constexpr float ATTACK_TIME = 50.0f;
        constexpr float RELEASE_TIME = 200.0f;
        constexpr float TOLERANCE = 0.5f;

        constexpr bool DELTA_ENABLED = false;
        constexpr float DELTA_GAIN = 0.0f;
        constexpr bool DELTA_SOLO = false;

        constexpr int LATENCY_OFFSET = 0;
    }

    namespace ParamRanges
    {
        constexpr float GAIN_MIN = -40.0f;
        constexpr float GAIN_MAX = 40.0f;

        constexpr float ATTACK_MIN = 10.0f;
        constexpr float ATTACK_MAX = 500.0f;

        constexpr float RELEASE_MIN = 50.0f;
        constexpr float RELEASE_MAX = 2000.0f;

        constexpr float TOLERANCE_MIN = 0.1f;
        constexpr float TOLERANCE_MAX = 3.0f;

        constexpr int LATENCY_OFFSET_MIN = 0;
        constexpr int LATENCY_OFFSET_MAX = 48000;
    }

    enum class InstanceMode
    {
        Before = 0,
        After = 1
    };

    enum class MeasurementMode
    {
        RMS = 0,
        Peak = 1
    };

    enum class RMSWindow
    {
        Ms50 = 0,
        Ms100 = 1,
        Ms300 = 2
    };

    inline int rmsWindowToSamples(RMSWindow window, double sampleRate)
    {
        switch (window)
        {
            case RMSWindow::Ms50:  return static_cast<int>(sampleRate * 0.050);
            case RMSWindow::Ms100: return static_cast<int>(sampleRate * 0.100);
            case RMSWindow::Ms300: return static_cast<int>(sampleRate * 0.300);
            default:               return static_cast<int>(sampleRate * 0.100);
        }
    }

    inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{ ParamIDs::MODE, 1 },
            "Mode",
            juce::StringArray{ "Before", "After" },
            ParamDefaults::MODE));

        params.push_back(std::make_unique<juce::AudioParameterInt>(
            juce::ParameterID{ ParamIDs::PAIR_ID, 1 },
            "Pair ID",
            1, 16,
            ParamDefaults::PAIR_ID));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ ParamIDs::INPUT_GAIN, 1 },
            "Input Gain",
            juce::NormalisableRange<float>(ParamRanges::GAIN_MIN, ParamRanges::GAIN_MAX, 0.1f),
            ParamDefaults::INPUT_GAIN,
            juce::AudioParameterFloatAttributes().withLabel("dB")));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ ParamIDs::OUTPUT_GAIN, 1 },
            "Output Gain",
            juce::NormalisableRange<float>(ParamRanges::GAIN_MIN, ParamRanges::GAIN_MAX, 0.1f),
            ParamDefaults::OUTPUT_GAIN,
            juce::AudioParameterFloatAttributes().withLabel("dB")));

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{ ParamIDs::BYPASS, 1 },
            "Bypass",
            ParamDefaults::BYPASS));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{ ParamIDs::MEASUREMENT_MODE, 1 },
            "Measurement Mode",
            juce::StringArray{ "RMS", "Peak" },
            ParamDefaults::MEASUREMENT_MODE));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{ ParamIDs::RMS_WINDOW, 1 },
            "RMS Window",
            juce::StringArray{ "50ms", "100ms", "300ms" },
            ParamDefaults::RMS_WINDOW));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ ParamIDs::ATTACK_TIME, 1 },
            "Attack Time",
            juce::NormalisableRange<float>(ParamRanges::ATTACK_MIN, ParamRanges::ATTACK_MAX, 1.0f, 0.5f),
            ParamDefaults::ATTACK_TIME,
            juce::AudioParameterFloatAttributes().withLabel("ms")));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ ParamIDs::RELEASE_TIME, 1 },
            "Release Time",
            juce::NormalisableRange<float>(ParamRanges::RELEASE_MIN, ParamRanges::RELEASE_MAX, 1.0f, 0.5f),
            ParamDefaults::RELEASE_TIME,
            juce::AudioParameterFloatAttributes().withLabel("ms")));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ ParamIDs::TOLERANCE, 1 },
            "Tolerance",
            juce::NormalisableRange<float>(ParamRanges::TOLERANCE_MIN, ParamRanges::TOLERANCE_MAX, 0.1f),
            ParamDefaults::TOLERANCE,
            juce::AudioParameterFloatAttributes().withLabel("dB")));

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{ ParamIDs::DELTA_ENABLED, 1 },
            "Delta Enable",
            ParamDefaults::DELTA_ENABLED));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ ParamIDs::DELTA_GAIN, 1 },
            "Delta Gain",
            juce::NormalisableRange<float>(ParamRanges::GAIN_MIN, ParamRanges::GAIN_MAX, 0.1f),
            ParamDefaults::DELTA_GAIN,
            juce::AudioParameterFloatAttributes().withLabel("dB")));

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{ ParamIDs::DELTA_SOLO, 1 },
            "Delta Solo",
            ParamDefaults::DELTA_SOLO));

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{ ParamIDs::LISTEN_BEFORE, 1 },
            "Listen Before",
            false));

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{ ParamIDs::LISTEN_AFTER, 1 },
            "Listen After",
            false));

        params.push_back(std::make_unique<juce::AudioParameterInt>(
            juce::ParameterID{ ParamIDs::LATENCY_OFFSET, 1 },
            "Latency Offset",
            ParamRanges::LATENCY_OFFSET_MIN,
            ParamRanges::LATENCY_OFFSET_MAX,
            ParamDefaults::LATENCY_OFFSET));

        return { params.begin(), params.end() };
    }
}
