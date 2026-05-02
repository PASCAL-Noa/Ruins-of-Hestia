#include "StaticVoice.h"
#include <algorithm>
#include "AudioMath.h"

namespace GPC
{
    StaticVoice::StaticVoice(std::span<const float> samples, uint32_t channels)
        : m_Data(samples), m_Channels(channels) {}

    void StaticVoice::PullSamples(std::span<float> destinationMix)
    {
        if (m_Finished.load(std::memory_order_relaxed) || m_Data.empty())
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
        size_t framesWritten = 0;

        while (framesWritten < frameCount)
        {
            const size_t availableFrames = (m_Data.size() - m_Cursor) / m_Channels;

            if (availableFrames == 0)
            {
                if (loop)
                {
                    m_Cursor = 0;
                        continue;
                }
                m_Finished.store(true, std::memory_order_relaxed);
                break;
            }

            const size_t framesToProcess = std::min(frameCount - framesWritten, availableFrames);
            AudioMath::StereoDestination chunk = destinationMix.subspan(framesWritten * 2, framesToProcess * 2);
            std::span<const float> source(m_Data.data() + m_Cursor, framesToProcess * m_Channels);

            AudioMath::ProcessMixSlewed(chunk, source, m_Channels, m_CurrentVol, targetVol);

            m_Cursor += framesToProcess * m_Channels;
            framesWritten += framesToProcess;
        }
        if (framesWritten < frameCount)
            std::ranges::fill(destinationMix.subspan(framesWritten * 2), 0.0f);
    }

    bool StaticVoice::IsFinished() const
    {
        return m_Finished.load(std::memory_order_relaxed);
    }

    void StaticVoice::Stop()
    {
        m_Finished.store(true, std::memory_order_relaxed);
    }

    void StaticVoice::SetVolume(AudioMath::StereoVolume volume)
    {
        m_VolumeLeft.store(volume.Left, std::memory_order_relaxed);
        m_VolumeRight.store(volume.Right, std::memory_order_relaxed);
    }

    void StaticVoice::SetLooping(bool loop)
    {
        m_Looping.store(loop, std::memory_order_relaxed);
    }
}