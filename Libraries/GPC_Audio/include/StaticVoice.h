#pragma once
#include "IAudioVoice.h"
#include <span>
#include <atomic>

namespace GPC
{
    class StaticVoice : public IAudioVoice
    {
    public:
        StaticVoice(std::span<const float> samples, uint32_t channels);

        void                        PullSamples(std::span<float> destinationMix) override;
        bool                        IsFinished() const override;
        void                        Stop() override;
        void                        SetVolume(AudioMath::StereoVolume volume) override;
        void                        SetLooping(bool loop) override;

    private:
        std::span<const float>      m_Data;
        uint32_t                    m_Channels;
        size_t                      m_Cursor = 0;
        std::atomic<float>          m_VolumeLeft{1.0f};
        std::atomic<float>          m_VolumeRight{1.0f};
        std::atomic<bool>           m_Looping{false};
        std::atomic<bool>           m_Finished{false};
        AudioMath::StereoVolume     m_CurrentVol{-1.0f, -1.0f};
    };
}