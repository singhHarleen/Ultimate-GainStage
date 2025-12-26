#pragma once

#include <JuceHeader.h>
#include <atomic>
#include <array>
#include <map>
#include <mutex>

namespace GainStage
{
    constexpr int kMaxPairIDs = 16;
    constexpr int kMaxChannels = 2;
    constexpr float kBufferLengthSeconds = 1.0f;

    struct SharedAudioData
    {
        static constexpr int kDefaultBufferSize = 48000;

        std::array<std::vector<float>, kMaxChannels> channelBuffers;
        std::atomic<int> writePosition{ 0 };
        std::atomic<uint64_t> writeSequence{ 0 };
        std::atomic<double> sampleRate{ 48000.0 };
        std::atomic<int> numChannels{ 2 };
        std::atomic<bool> beforeInstanceActive{ false };
        std::atomic<uint64_t> lastWriteTime{ 0 };

        int bufferSize = kDefaultBufferSize;

        SharedAudioData()
        {
            for (auto& ch : channelBuffers)
                ch.resize(kDefaultBufferSize, 0.0f);
        }

        void resize(int newSize, int channels)
        {
            bufferSize = newSize;
            numChannels.store(channels);
            for (int ch = 0; ch < channels && ch < kMaxChannels; ++ch)
                channelBuffers[ch].resize(newSize, 0.0f);
        }

        void clear()
        {
            for (auto& ch : channelBuffers)
                std::fill(ch.begin(), ch.end(), 0.0f);
            writePosition.store(0);
            writeSequence.store(0);
        }
    };

    class SharedBufferManager
    {
    public:
        static SharedBufferManager& getInstance()
        {
            static SharedBufferManager instance;
            return instance;
        }

        SharedAudioData& getBuffer(int pairID)
        {
            jassert(pairID >= 1 && pairID <= kMaxPairIDs);
            return buffers_[pairID - 1];
        }

        void writeSamples(int pairID, const juce::AudioBuffer<float>& source, int numSamples)
        {
            if (pairID < 1 || pairID > kMaxPairIDs)
                return;

            auto& data = buffers_[pairID - 1];
            const int numChannels = juce::jmin(source.getNumChannels(), kMaxChannels);
            const int bufSize = data.bufferSize;

            int writePos = data.writePosition.load(std::memory_order_relaxed);

            for (int ch = 0; ch < numChannels; ++ch)
            {
                const float* src = source.getReadPointer(ch);
                auto& dest = data.channelBuffers[ch];

                int pos = writePos;
                for (int i = 0; i < numSamples; ++i)
                {
                    dest[pos] = src[i];
                    pos = (pos + 1) % bufSize;
                }
            }

            int newWritePos = (writePos + numSamples) % bufSize;
            data.writePosition.store(newWritePos, std::memory_order_release);
            data.writeSequence.fetch_add(1, std::memory_order_release);
            data.lastWriteTime.store(juce::Time::currentTimeMillis(), std::memory_order_release);
            data.beforeInstanceActive.store(true, std::memory_order_release);
        }

        void readSamples(int pairID, juce::AudioBuffer<float>& dest, int numSamples, int latencyOffset = 0)
        {
            if (pairID < 1 || pairID > kMaxPairIDs)
                return;

            auto& data = buffers_[pairID - 1];
            const int numChannels = juce::jmin(dest.getNumChannels(), data.numChannels.load());
            const int bufSize = data.bufferSize;

            int writePos = data.writePosition.load(std::memory_order_acquire);

            int readPos = writePos - numSamples - latencyOffset;
            while (readPos < 0)
                readPos += bufSize;
            readPos = readPos % bufSize;

            for (int ch = 0; ch < numChannels; ++ch)
            {
                float* destPtr = dest.getWritePointer(ch);
                const auto& src = data.channelBuffers[ch];

                int pos = readPos;
                for (int i = 0; i < numSamples; ++i)
                {
                    destPtr[i] = src[pos];
                    pos = (pos + 1) % bufSize;
                }
            }
        }

        bool isBeforeInstanceActive(int pairID) const
        {
            if (pairID < 1 || pairID > kMaxPairIDs)
                return false;

            const auto& data = buffers_[pairID - 1];

            if (!data.beforeInstanceActive.load(std::memory_order_acquire))
                return false;

            uint64_t lastWrite = data.lastWriteTime.load(std::memory_order_acquire);
            uint64_t now = juce::Time::currentTimeMillis();

            return (now - lastWrite) < 1000;
        }

        void setBeforeInstanceInactive(int pairID)
        {
            if (pairID >= 1 && pairID <= kMaxPairIDs)
                buffers_[pairID - 1].beforeInstanceActive.store(false, std::memory_order_release);
        }

        void prepareBuffer(int pairID, double sampleRate, int numChannels)
        {
            if (pairID < 1 || pairID > kMaxPairIDs)
                return;

            auto& data = buffers_[pairID - 1];
            int newSize = static_cast<int>(sampleRate * kBufferLengthSeconds);
            data.resize(newSize, numChannels);
            data.sampleRate.store(sampleRate);
            data.clear();
        }

    private:
        SharedBufferManager() = default;
        ~SharedBufferManager() = default;

        SharedBufferManager(const SharedBufferManager&) = delete;
        SharedBufferManager& operator=(const SharedBufferManager&) = delete;

        std::array<SharedAudioData, kMaxPairIDs> buffers_;
    };

}
