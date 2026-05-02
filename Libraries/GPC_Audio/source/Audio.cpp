#include "Audio.h"
#include "Debugger.h"

namespace GPC
{
    Audio::Audio() : m_Created(false), m_SampleRate(0), m_BufferFrames(0), m_Channels(0)
    {}

    Audio::~Audio()
    {
        try
        {
            Stop();
        }
        catch (const std::exception& e)
        {
            GPC_ERROR << "Failed to close RtAudio stream: " << e.what() << ENDL;
        }
    }

    bool Audio::Create(const AudioConfig &config)
    {
        if (m_Created)
            return false;

        m_SampleRate = config.SampleRate;
        m_BufferFrames = config.BufferFrames;
        m_Channels = config.Channels;

        m_RingBuffer.resize(m_BufferFrames * m_Channels * config.RingBufferSizeFactor);
        std::ranges::fill(m_RingBuffer, 0.0f);

        if (m_RtAudio.getDeviceCount() < 1)
            return false;

        RtAudio::StreamParameters parameters;
        parameters.deviceId = m_RtAudio.getDefaultOutputDevice();
        parameters.nChannels = m_Channels;
        parameters.firstChannel = 0;

        try
        {
            m_RtAudio.openStream(&parameters, nullptr, RTAUDIO_FLOAT32,
                                 m_SampleRate, &m_BufferFrames, &Callback, this);
            m_Created = true;
            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    void Audio::Start()
    {
        if (m_RtAudio.isStreamOpen() && !m_RtAudio.isStreamRunning())
            m_RtAudio.startStream();
    }

    void Audio::Stop()
    {
        if (m_RtAudio.isStreamRunning())
            m_RtAudio.stopStream();
    }

    int Audio::Callback(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames,
                        double streamTime, RtAudioStreamStatus status, void* userData)
    {
        float* out = static_cast<float *>(outputBuffer);
        Audio* self = static_cast<Audio*>(userData);

        size_t readIdx = self->m_ReadPtr.load(std::memory_order_relaxed);
        size_t writeIdx = self->m_WritePtr.load(std::memory_order_acquire);
        size_t capacity = self->m_RingBuffer.size();
        uint32_t samplesToRead = nBufferFrames * self->m_Channels;

        for (uint32_t i = 0; i < samplesToRead; ++i)
        {
            if (readIdx != writeIdx)
            {
                out[i] = self->m_RingBuffer[readIdx];
                readIdx = (readIdx + 1) % capacity;
            }
            else
            {
                out[i] = 0.0f;
            }
        }

        self->m_ReadPtr.store(readIdx, std::memory_order_release);
        return 0;
    }

    bool Audio::PushSamples(std::span<const float> samples)
    {
        size_t writeIdx = m_WritePtr.load(std::memory_order_relaxed);
        size_t readIdx = m_ReadPtr.load(std::memory_order_acquire);
        size_t capacity = m_RingBuffer.size();
        size_t freeSpace = (capacity + readIdx - writeIdx - 1) % capacity;

        if (samples.size() > freeSpace)
            return false;

        for (float sample : samples)
        {
            m_RingBuffer[writeIdx] = sample;
            writeIdx = (writeIdx + 1) % capacity;
        }

        m_WritePtr.store(writeIdx, std::memory_order_release);
        return true;
    }
}