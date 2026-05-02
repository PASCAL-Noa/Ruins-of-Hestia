#include "WavParser.h"

#include <cstring>
#include "AudioMath.h"

namespace GPC
{
    bool WavParser::IsId(const char* a, const char* b)
    {
        return std::memcmp(a, b, 4) == 0;
    }

    bool WavParser::Open(const std::string& path)
    {
        if (m_File.is_open()) m_File.close();

        m_File.open(path, std::ios::binary);
        if (!m_File.is_open()) return false;

        if (!ReadRIFFHeader()) return false;

        uint32_t chunkSize = 0;
        m_File.seekg(12, std::ios::beg);

        if (!FindChunk("fmt ", chunkSize))    return false;
        if (!ReadFmtChunk(chunkSize))               return false;
        if (!LocateDataChunk())                     return false;

        return true;
    }

    void WavParser::Close()
    {
        if (m_File.is_open()) m_File.close();
    }

    bool WavParser::ReadRIFFHeader()
    {
        m_File.seekg(0, std::ios::beg);
        m_File.read(reinterpret_cast<char*>(&m_Header), sizeof(RIFFHeader));
        return (IsId(m_Header.CkID, "RIFF") && IsId(m_Header.WAVEID, "WAVE"));
    }

    bool WavParser::FindChunk(const char id[4], uint32_t& chunkSize)
    {
        while (m_File)
        {
            char curId[4];
            m_File.read(curId, 4);
            if (!m_File) return false;

            uint32_t size = 0;
            m_File.read(reinterpret_cast<char*>(&size), sizeof(size));
            if (!m_File) return false;

            if (IsId(curId, id))
            {
                chunkSize = size;
                return true;
            }

            m_File.seekg(size, std::ios::cur);
            if (size % 2 == 1) m_File.seekg(1, std::ios::cur);
        }
        return false;
    }

    bool WavParser::ReadFmtChunk(uint32_t chunkSize)
    {
        if (chunkSize < 16) return false;

        std::memcpy(m_Fmt.CkID, "fmt ", 4);
        m_Fmt.CkSize = chunkSize;

        m_File.read(reinterpret_cast<char*>(&m_Fmt.WFormatTag), 16);

        int64_t remaining = static_cast<int64_t>(chunkSize) - 16;
        if (remaining > 0)
            m_File.seekg(remaining, std::ios::cur);

        if (!m_File) return false;

        m_FmtValid = true;
        return true;
    }

    bool WavParser::LocateDataChunk()
    {
        m_File.seekg(12, std::ios::beg);
        uint32_t chunkSize = 0;
        if (!FindChunk("data", chunkSize)) return false;

        m_DataSize = chunkSize;
        m_DataStartPos = m_File.tellg();
        m_BytesRead = 0;
        return true;
    }

    uint16_t WavParser::GetChannels() const
    {
        return m_Fmt.NChannels;
    }

    uint32_t WavParser::GetSampleRate() const
    {
        return m_Fmt.NSamplesPerSec;
    }

    void WavParser::ResetPosition()
    {
        if (m_File.is_open())
        {
            m_File.seekg(m_DataStartPos, std::ios::beg);
            m_BytesRead = 0;
        }
    }

    bool WavParser::LoadToMemory(const std::string& path, AudioResourceAsset& outResource)
    {
        WavParser parser;
        if (!parser.Open(path)) return false;

        uint32_t totalSamples = AudioMath::BytesToSampleCount(parser.m_DataSize, parser.m_Fmt.WBitsPerSample);
        uint32_t totalFrames  = AudioMath::BytesToFrameCount(parser.m_DataSize, parser.m_Fmt.NBlockAlign);

        outResource.Samples.resize(totalSamples);
        outResource.SampleRate = parser.m_Fmt.NSamplesPerSec;
        outResource.Channels   = parser.m_Fmt.NChannels;
        outResource.Duration   = AudioMath::CalculateDuration(totalFrames, outResource.SampleRate);

        return parser.ReadAndConvert(outResource.Samples, parser.m_DataSize, totalSamples);
    }

    size_t WavParser::ReadFrames(std::span<float> outBuffer, size_t framesToRead)
    {
        if (!m_File.is_open() || !m_FmtValid || outBuffer.empty()) return 0;

        const uint32_t blockAlign = m_Fmt.NBlockAlign;
        const uint32_t remainingBytes = m_DataSize - m_BytesRead;
        const size_t framesAvailable = remainingBytes / blockAlign;
        const size_t framesToProcess = std::min(framesToRead, framesAvailable);

        if (framesToProcess == 0) return 0;

        const size_t bytesToRead = framesToProcess * blockAlign;
        const size_t samplesToProcess = framesToProcess * m_Fmt.NChannels;

        if (!ReadAndConvert(outBuffer.subspan(0, samplesToProcess), bytesToRead, samplesToProcess))
            return 0;

        m_BytesRead += bytesToRead;
        return framesToProcess;
    }



    bool WavParser::ReadAndConvert(std::span<float> outBuffer, size_t bytesToRead, size_t samplesToProcess)
    {
        if (m_Fmt.WFormatTag == 1 && m_Fmt.WBitsPerSample == 16)
        {
            std::vector<int16_t> temp(samplesToProcess);
            m_File.read(reinterpret_cast<char*>(temp.data()), bytesToRead);

            for (size_t i = 0; i < samplesToProcess; ++i)
                outBuffer[i] = AudioMath::NormalizeSample(temp[i]);

            return true;
        }

        if (m_Fmt.WFormatTag == 3 && m_Fmt.WBitsPerSample == 32)
        {
            m_File.read(reinterpret_cast<char*>(outBuffer.data()), bytesToRead);
            return true;
        }

        return false;
    }

}
