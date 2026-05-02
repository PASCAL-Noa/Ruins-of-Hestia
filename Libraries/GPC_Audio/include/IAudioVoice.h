#pragma once
#include <span>
#include "AudioMath.h"

namespace GPC
{
    class IAudioVoice
    {
    public:
        virtual ~IAudioVoice() = default;

        virtual void    PullSamples(std::span<float> destinationMix) = 0;
        virtual bool    IsFinished() const = 0;
        virtual void    Stop() = 0;
        virtual void    SetVolume(AudioMath::StereoVolume volume) = 0;
        virtual void    SetLooping(bool loop) = 0;
    };
}
