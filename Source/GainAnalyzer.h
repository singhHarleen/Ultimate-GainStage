#pragma once

#include <JuceHeader.h>
#include <vector>
#include <cmath>

namespace GainStage
{
    class GainAnalyzer
    {
    public:
        GainAnalyzer() = default;

        void prepare(double sampleRate, int maxBlockSize)
        {
            sampleRate_ = sampleRate;
            rmsBuffer_.resize(static_cast<size_t>(sampleRate * 0.5), 0.0f);
            rmsWritePos_ = 0;
            currentRMS_ = 0.0f;
            currentPeak_ = 0.0f;
            peakHoldCounter_ = 0;
        }

        void setRMSWindowSamples(int samples)
        {
            rmsWindowSamples_ = juce::jlimit(1, static_cast<int>(rmsBuffer_.size()), samples);
        }

        void setPeakHoldSamples(int samples)
        {
            peakHoldSamples_ = samples;
        }

        void process(const juce::AudioBuffer<float>& buffer)
        {
            const int numChannels = buffer.getNumChannels();
            const int numSamples = buffer.getNumSamples();

            float blockPeak = 0.0f;
            double blockSumSquares = 0.0;

            for (int ch = 0; ch < numChannels; ++ch)
            {
                const float* data = buffer.getReadPointer(ch);

                for (int i = 0; i < numSamples; ++i)
                {
                    const float sample = data[i];
                    const float absSample = std::abs(sample);

                    blockPeak = juce::jmax(blockPeak, absSample);
                    blockSumSquares += static_cast<double>(sample * sample);

                    rmsBuffer_[rmsWritePos_] = sample * sample;
                    rmsWritePos_ = (rmsWritePos_ + 1) % rmsBuffer_.size();
                }
            }

            if (blockPeak >= currentPeak_)
            {
                currentPeak_ = blockPeak;
                peakHoldCounter_ = peakHoldSamples_;
            }
            else if (peakHoldCounter_ > 0)
            {
                peakHoldCounter_ -= numSamples;
            }
            else
            {
                currentPeak_ = blockPeak;
            }

            double sum = 0.0;
            const int windowSize = rmsWindowSamples_;
            int pos = static_cast<int>(rmsWritePos_) - windowSize;
            if (pos < 0) pos += static_cast<int>(rmsBuffer_.size());

            for (int i = 0; i < windowSize; ++i)
            {
                sum += rmsBuffer_[(pos + i) % rmsBuffer_.size()];
            }

            currentRMS_ = static_cast<float>(std::sqrt(sum / (windowSize * numChannels)));
        }

        float getRMSLevel() const { return currentRMS_; }
        float getPeakLevel() const { return currentPeak_; }

        float getRMSdB() const
        {
            return (currentRMS_ > 0.0f) ? 20.0f * std::log10(currentRMS_) : -100.0f;
        }

        float getPeakdB() const
        {
            return (currentPeak_ > 0.0f) ? 20.0f * std::log10(currentPeak_) : -100.0f;
        }

    private:
        double sampleRate_ = 48000.0;
        std::vector<float> rmsBuffer_;
        size_t rmsWritePos_ = 0;
        int rmsWindowSamples_ = 4800;
        int peakHoldSamples_ = 4800;
        int peakHoldCounter_ = 0;
        float currentRMS_ = 0.0f;
        float currentPeak_ = 0.0f;
    };

    class GainSmoother
    {
    public:
        void prepare(double sampleRate)
        {
            sampleRate_ = sampleRate;
            updateCoefficients();
        }

        void setAttackTime(float ms)
        {
            attackMs_ = ms;
            updateCoefficients();
        }

        void setReleaseTime(float ms)
        {
            releaseMs_ = ms;
            updateCoefficients();
        }

        float process(float targetGaindB)
        {
            if (targetGaindB < currentGaindB_)
            {
                currentGaindB_ = attackCoeff_ * currentGaindB_ + (1.0f - attackCoeff_) * targetGaindB;
            }
            else
            {
                currentGaindB_ = releaseCoeff_ * currentGaindB_ + (1.0f - releaseCoeff_) * targetGaindB;
            }

            return currentGaindB_;
        }

        float getCurrentGaindB() const { return currentGaindB_; }

        void reset()
        {
            currentGaindB_ = 0.0f;
        }

    private:
        void updateCoefficients()
        {
            if (sampleRate_ <= 0.0) return;

            attackCoeff_ = std::exp(-1.0f / (static_cast<float>(sampleRate_) * attackMs_ * 0.001f));
            releaseCoeff_ = std::exp(-1.0f / (static_cast<float>(sampleRate_) * releaseMs_ * 0.001f));
        }

        double sampleRate_ = 48000.0;
        float attackMs_ = 50.0f;
        float releaseMs_ = 200.0f;
        float attackCoeff_ = 0.99f;
        float releaseCoeff_ = 0.999f;
        float currentGaindB_ = 0.0f;
    };
}
