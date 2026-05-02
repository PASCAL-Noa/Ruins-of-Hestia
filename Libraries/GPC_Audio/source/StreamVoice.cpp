#include "StreamVoice.h"
#include <algorithm>
#include "AudioMath.h"

namespace GPC
{
    StreamVoice::StreamVoice(const std::string& path)
    {
        if (m_Parser.Open(path))
        {
            m_Channels = m_Parser.GetChannels();
            m_StreamBuffer.resize(8192 * m_Channels, 0.0f);
        }
        else
        {
            m_Finished.store(true, std::memory_order_relaxed);
        }
    }

    void StreamVoice::PullSamples(std::span<float> destinationMix)
    {
        if (m_Finished.load(std::memory_order_relaxed))
        {
            std::ranges::fill(destinationMix, 0.0f);
                return;
        }

        const bool loop = m_Looping.load(std::memory_order_relaxed);
        AudioMath::StereoVolume targetVol = {
            m_VolumeLeft.load(std::memory_order_relaxed),
            m_VolumeRight.load(std::memory_order_relaxed)
        };

        if (m_CurrentVol.Left < 0.0f)
        {
            m_CurrentVol = targetVol;
        }
        const size_t frameCount = destinationMix.size() / 2;
        const size_t samplesNeeded = frameCount * m_Channels;

        if (m_StreamBuffer.size() < samplesNeeded)
            m_StreamBuffer.resize(samplesNeeded, 0.0f);

        std::span<float> readTarget(m_StreamBuffer.data(), samplesNeeded);
        size_t framesRead = m_Parser.ReadFrames(readTarget, frameCount);

        if (framesRead < frameCount)
        {
            if (loop)
            {
            m_Parser.ResetPosition();
                size_t remainingFrames = frameCount - framesRead;
                size_t offset = framesRead * m_Channels;

                std::span<float> remainTarget(m_StreamBuffer.data() + offset, remainingFrames * m_Channels);
                framesRead += m_Parser.ReadFrames(remainTarget, remainingFrames);
            }
            else
            {
                m_Finished.store(true, std::memory_order_relaxed);
            }
        }

        AudioMath::StereoDestination chunk = destinationMix.subspan(0, framesRead * 2);
        std::span<const float> source(m_StreamBuffer.data(), framesRead * m_Channels);

        AudioMath::ProcessMixSlewed(chunk, source, m_Channels, m_CurrentVol, targetVol);

        if (framesRead < frameCount)
            std::ranges::fill(destinationMix.subspan(framesRead * 2), 0.0f);
    }

    bool StreamVoice::IsFinished() const
    {
        return m_Finished.load(std::memory_order_relaxed);
    }

    void StreamVoice::Stop()
    {
        m_Finished.store(true, std::memory_order_relaxed);
    }

    void StreamVoice::SetVolume(AudioMath::StereoVolume volume)
    {
        m_VolumeLeft.store(volume.Left, std::memory_order_relaxed);
        m_VolumeRight.store(volume.Right, std::memory_order_relaxed);
    }

    void StreamVoice::SetLooping(bool loop)
    {
        m_Looping.store(loop, std::memory_order_relaxed);
    }
}