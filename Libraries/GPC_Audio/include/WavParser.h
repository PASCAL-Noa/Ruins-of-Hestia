#pragma once

#include <cstdint>
#include <fstream>
#include <span>
#include <string>
#include <vector>

#include "../../GPC_Assets/include/Assets.h"

namespace GPC
{

#pragma pack(push, 1)
    struct RIFFHeader
    {
        char CkID[4];       // "RIFF"
        uint32_t CkSize;
        char WAVEID[4];     // "WAVE"
    };

    struct FmtChunk
    {
        char CkID[4];       // "fmt "
        uint32_t CkSize;
        uint16_t WFormatTag;
        uint16_t NChannels;
        uint32_t NSamplesPerSec;
        uint32_t NAvgBytesPerSec;
        uint16_t NBlockAlign;
        uint16_t WBitsPerSample;
    };
#pragma pack(pop)

    struct WavHeaderInfo
    {
        FmtChunk Format;
        uint32_t DataSize;
        std::streampos DataStartPos;
    };

    class WavParser
    {
    public:
        WavParser() = default;
        ~WavParser() { Close(); }

        bool            Open(const std::string& path);
        void            Close();

        uint16_t        GetChannels() const;
        uint32_t        GetSampleRate() const;
        void            ResetPosition();

        size_t          ReadFrames(std::span<float> outBuffer, size_t framesToRead);
        static bool     LoadToMemory(const std::string& path, AudioResourceAsset& outResource);

    private:
        std::ifstream   m_File;
        RIFFHeader      m_Header{};
        FmtChunk        m_Fmt{};
        bool            m_FmtValid{false};

        uint32_t        m_DataSize{0};
        std::streampos  m_DataStartPos{0};
        uint32_t        m_BytesRead{0};

        bool            IsId(const char* a, const char* b);
        bool            ReadRIFFHeader();
        bool            FindChunk(const char id[4], uint32_t& chunkSize);
        bool            ReadFmtChunk(uint32_t chunkSize);
        bool            LocateDataChunk();
        bool            ReadAndConvert(std::span<float> outBuffer, size_t bytesToRead, size_t samplesToProcess);
    };
}
