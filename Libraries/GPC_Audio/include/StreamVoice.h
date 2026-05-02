#pragma once
#include "IAudioVoice.h"
#include "WavParser.h"
#include <span>
#include <atomic>
#include <string>
#include <vector>

namespace GPC
{
    class StreamVoice : public IAudioVoice
    {
    public:
        StreamVoice(const std::string& path);

        void                    PullSamples(std::span<float> outBuffer) override;
        bool                    IsFinished() const override;
        void                    Stop() override;
        void                    SetVolume(AudioMath::StereoVolume volume) override;
        void                    SetLooping(bool loop) override;

    private:
        WavParser               m_Parser;
        std::vector<float>      m_StreamBuffer;
        uint32_t                m_Channels{0};
        std::atomic<float>      m_VolumeLeft{1.0f};
        std::atomic<float>      m_VolumeRight{1.0f};
        std::atomic<bool>       m_Looping{false};
        std::atomic<bool>       m_Finished{false};
        AudioMath::StereoVolume m_CurrentVol{-1.0f, -1.0f};
    };
}
