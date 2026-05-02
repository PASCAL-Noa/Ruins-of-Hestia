#include "AudioMixer.h"
#include <algorithm>

namespace GPC
{
    void AudioMixer::Mix(std::span<float> destinationMix)
    {
        std::ranges::fill(destinationMix, 0.0f);

        if (m_VoiceBuffer.size() < destinationMix.size())
            m_VoiceBuffer.resize(destinationMix.size(), 0.0f);

        std::span<float> voiceSpan(m_VoiceBuffer.data(), destinationMix.size());

        for (IAudioVoice* voice : m_Voices)
        {
            voice->PullSamples(voiceSpan);

            for (size_t i = 0; i < destinationMix.size(); ++i)
                destinationMix[i] += voiceSpan[i];
        }

        for (float& sample : destinationMix)
            sample = std::clamp(sample * m_MasterVolume, -1.0f, 1.0f);
    }

    void AudioMixer::AddVoice(IAudioVoice* voice)
    {
        m_Voices.push_back(voice);
    }

    void AudioMixer::RemoveVoice(IAudioVoice* voice)
    {
        std::erase(m_Voices, voice);
    }

    void AudioMixer::SetMasterVolume(float volume)
    {
        m_MasterVolume = volume;
    }
}