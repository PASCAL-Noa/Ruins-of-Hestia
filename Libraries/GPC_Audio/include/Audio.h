#pragma once

#include <atomic>
#include <span>
#include "RtAudio/RtAudio.h"

namespace GPC
{
    struct AudioConfig
    {
        uint32_t SampleRate = 44100;
        uint32_t BufferFrames = 256;
        uint32_t Channels = 2;
        uint32_t RingBufferSizeFactor = 8;
    };

    class Audio
    {
    public:
        explicit Audio();
        ~Audio();

        Audio(const Audio&) = delete;
        Audio& operator=(const Audio&) = delete;

        bool                    Create(const AudioConfig& config);
        void                    Start();
        void                    Stop();

        bool                    PushSamples(std::span<const float> samples);

    private:
        bool                    m_Created = false;
        RtAudio                 m_RtAudio;
        uint32_t                m_SampleRate;
        uint32_t                m_BufferFrames;
        uint32_t                m_Channels;

        std::vector<float>      m_RingBuffer;
        std::atomic<size_t>     m_WritePtr{0};
        std::atomic<size_t>     m_ReadPtr{0};

        static int              Callback(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames,
                                        double streamTime, RtAudioStreamStatus status, void* userData);
    };
}
