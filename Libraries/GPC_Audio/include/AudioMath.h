#pragma once
#include <cstdint>
#include <span>
#include <algorithm>

namespace GPC::AudioMath
{
    using MonoBuffer = std::span<const float>;
    using StereoBuffer = std::span<const float>;
    using StereoDestination = std::span<float>;

    struct StereoVolume
    {
        float Left = 1.0f;
        float Right = 1.0f;
    };

    constexpr float NormalizeSample(int16_t sample)
    {
        return static_cast<float>(sample) / 32768.0f;
    }

    constexpr uint32_t BytesToSampleCount(uint32_t dataSize, uint16_t bitsPerSample)
    {
        return dataSize / (bitsPerSample / 8);
    }

    constexpr uint32_t BytesToFrameCount(uint32_t dataSize, uint16_t blockAlign)
    {
        return dataSize / blockAlign;
    }

    constexpr float CalculateDuration(uint32_t frameCount, uint32_t sampleRate)
    {
        return static_cast<float>(frameCount) / static_cast<float>(sampleRate);
    }

    inline void ProcessMono(StereoDestination destinationMix, MonoBuffer sourceData, StereoVolume startVol, StereoVolume endVol)
    {
        const size_t frames = sourceData.size();
        if (frames == 0) return;

        const float stepLeft = (endVol.Left - startVol.Left) / static_cast<float>(frames);
        const float stepRight = (endVol.Right - startVol.Right) / static_cast<float>(frames);

        float currentLeft = startVol.Left;
        float currentRight = startVol.Right;

        for (size_t i = 0; i < frames; ++i)
        {
            const float sample = sourceData[i];
            destinationMix[i * 2 + 0] = sample * currentLeft;
            destinationMix[i * 2 + 1] = sample * currentRight;
            currentLeft += stepLeft;
            currentRight += stepRight;
        }
    }

    inline void ProcessStereo(StereoDestination destinationMix, StereoBuffer sourceData, StereoVolume startVol, StereoVolume endVol)
    {
        const size_t frames = sourceData.size() / 2;
        if (frames == 0) return;

        const float stepLeft = (endVol.Left - startVol.Left) / static_cast<float>(frames);
        const float stepRight = (endVol.Right - startVol.Right) / static_cast<float>(frames);

        float currentLeft = startVol.Left;
        float currentRight = startVol.Right;

        for (size_t i = 0; i < frames; ++i)
        {
            destinationMix[i * 2 + 0] = sourceData[i * 2 + 0] * currentLeft;
            destinationMix[i * 2 + 1] = sourceData[i * 2 + 1] * currentRight;
            currentLeft += stepLeft;
            currentRight += stepRight;
        }
    }

    inline void ProcessMix(StereoDestination destinationMix, std::span<const float> sourceData, uint32_t channels, StereoVolume startVol, StereoVolume endVol)
    {
        if (channels == 1)
            ProcessMono(destinationMix, sourceData, startVol, endVol);
        else
            ProcessStereo(destinationMix, sourceData, startVol, endVol);
    }

    constexpr float MAX_DELTA_PER_SAMPLE = 0.0005f;

    inline void ProcessMixSlewed(StereoDestination destinationMix, std::span<const float> sourceData, uint32_t channels, StereoVolume& currentVol, const StereoVolume& targetVol)
    {
        const size_t frames = (channels == 1) ? sourceData.size() : sourceData.size() / 2;
        if (frames == 0) return;

        const float maxDelta = static_cast<float>(frames) * MAX_DELTA_PER_SAMPLE;

        float endLeft = currentVol.Left + std::clamp(targetVol.Left - currentVol.Left, -maxDelta, maxDelta);
        float endRight = currentVol.Right + std::clamp(targetVol.Right - currentVol.Right, -maxDelta, maxDelta);

        StereoVolume endVol = { endLeft, endRight };

        ProcessMix(destinationMix, sourceData, channels, currentVol, endVol);
        currentVol = endVol;
    }
}