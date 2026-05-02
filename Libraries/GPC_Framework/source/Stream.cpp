//
// Created by hugoc on 03/03/2026.
//

#include "Stream.h"
#include "Debugger.h"
#ifndef GPC_LINUX
#include <share.h>
#endif

#include <cstdio>
#include <cstring>

#include "ArchiveManager.h"
#include "GPC_Framework.h"

namespace GPC
{
    Stream::Stream()
    {
        m_Path = "";
        m_OpenType = 0;
        m_FileByteSize = 0;
    }

    Stream::Stream(std::string_view path, uint32_t openType)
    {
        m_Path = path;
        m_OpenType = openType;
        m_FileByteSize = 0;
    }

    Stream::~Stream()
    {
        Close();
    }

    void Stream::OpenAs(const char* Mode)
    {
#ifndef GPC_LINUX
        mp_File = _fsopen(m_Path.c_str(), Mode, SH_DENYNO);
#else
        mp_File = fopen(m_Path.c_str(), Mode);
#endif
        if (mp_File == nullptr) {
            GPC_WARNING << "ERROR: Failed to open file : " << m_Path << ENDL;
            return;
        }
        m_IsOpen = true;

        CheckFileSize();
    }

    void Stream::Close()
    {
        if (m_IsOpen == false) return;
        if (fclose(mp_File) != 0) {
            GPC_WARNING << "ERROR: Failed to close file." << ENDL;
            return;
        }
        m_IsOpen = false;
    }

    const std::string& Stream::GetPath()
    {
        return m_Path;
    }

    uint64_t Stream::GetSize() const
    {
        return m_FileByteSize;
    }

    bool Stream::IsOpenType(FileOpenType Type) const
    {
        return m_IsOpen && (static_cast<uint32_t>(m_OpenType) & static_cast<uint32_t>(Type));
    }

    void Stream::PlaceCursor(uint64_t offset) const
    {
#ifndef GPC_LINUX
        auto err = _fseeki64(mp_File, static_cast<long long>(offset), SEEK_SET);
#else
        auto err = fseeko(mp_File, static_cast<off_t>(offset), SEEK_SET);
#endif
        if (err != 0) {
            GPC_WARNING << "Failed to seek to offset : " << offset << ENDL;
            return;
        }
    }

    int Stream::Get()
    {
        return fgetc(mp_File);
    }

    void Stream::Write(const void* pData, uint64_t byteSize)
    {
        DEBUG_ASSERT(m_IsOpen && mp_File != nullptr, "Tentative d'ecriture dans un fichier non ouvert !");
        if (fwrite(pData, byteSize, 1, mp_File) != 1)
            GPC_WARNING << "Failed to write to file." << ENDL;
    }

    void Stream::WriteAt(const void* pData, uint64_t offset, uint64_t byteSize)
    {
        PlaceCursor(offset);
        Write(pData, byteSize);
    }

    void Stream::WriteEnd(const void* pData, uint64_t byteSize)
    {
        SeekEnd();
        Write(pData, byteSize);
    }

    uint64_t Stream::ReadAll(uint8_t** pData)
    {
        DEBUG_ASSERT(pData != nullptr, "Tentative de lecture de fichier dans un pointeur nul !");

        if (*pData != nullptr) delete[] *pData;

        CheckFileSize();
        *pData = new uint8_t[m_FileByteSize];
        SeekStart();

        if (fread(*pData, m_FileByteSize, 1, mp_File) != 1)
            GPC_WARNING << "Failed to read in file." << ENDL;

        return m_FileByteSize;
    }

    void Stream::ReadAt(void* pData, uint64_t offset, uint64_t byteSize)
    {
        DEBUG_ASSERT(pData != nullptr, "Le buffer de destination (pData) ne peut pas etre nul !");

        PlaceCursor(offset);
        auto err = fread(pData, byteSize, 1, mp_File);
        if (err != 1) {
            GPC_WARNING << "Failed to read in File " << ENDL;
        }
    }

    void Stream::CheckFileSize()
    {
#ifndef GPC_LINUX
        _fseeki64(mp_File, 0, SEEK_END);
        m_FileByteSize = static_cast<uint64_t>(_ftelli64(mp_File));
#else
        if (fseeko(mp_File, 0, SEEK_END) != 0) {
            GPC_WARNING << "Failed to seek to end of file." << ENDL;
        } else {
            off_t size = ftello(mp_File);
            if (size < 0)
                GPC_WARNING << "Failed to get file size." << ENDL;
            else
                m_FileByteSize = static_cast<uint64_t>(size);
        }
#endif
        SeekStart();
    }

    void Stream::SeekEnd()
    {
#ifndef GPC_LINUX
        if (_fseeki64(mp_File, 0, SEEK_END) != 0)
            GPC_WARNING << "Failed to seek to end of file." << ENDL;
#else
        if (fseeko(mp_File, 0, SEEK_END) != 0)
            GPC_WARNING << "Failed to seek to end of file." << ENDL;
#endif
    }

    void Stream::SeekStart()
    {
#ifndef GPC_LINUX
        if (_fseeki64(mp_File, 0, SEEK_SET) != 0)
            GPC_WARNING << "Failed to seek to start of file." << ENDL;
#else
        if (fseeko(mp_File, 0, SEEK_SET) != 0)
            GPC_WARNING << "Failed to seek to start of file." << ENDL;
#endif
    }

    ReadStream::ReadStream(std::string_view path)
        : Stream(path, static_cast<uint32_t>(FileOpenType::READ))
    {
        OpenAs("rb");
    }

    WriteStream::WriteStream(std::string_view path)
        : Stream(path, static_cast<uint32_t>(FileOpenType::WRITE))
    {
        OpenAs("ab");
    }

    OverwriteStream::OverwriteStream(std::string_view path)
        : Stream(path, static_cast<uint32_t>(FileOpenType::WRITE | FileOpenType::DELETE))
    {
        OpenAs("wb");
    }

    ReadWriteStream::ReadWriteStream(std::string_view path)
        : Stream(path, static_cast<uint32_t>(FileOpenType::READ | FileOpenType::WRITE))
    {
        OpenAs("r+b");
    }

    ReadOverwriteStream::ReadOverwriteStream(std::string_view path)
        : Stream(path, static_cast<uint32_t>(FileOpenType::READ | FileOpenType::WRITE | FileOpenType::DELETE))
    {
        OpenAs("w+b");
    }

    uint32_t StreamUtils::CRC_32_From(Stream* pStream, uint64_t from)
    {
        DEBUG_ASSERT(pStream != nullptr, "Le pointeur pStream est nul !");

        pStream->PlaceCursor(from);
        uint32_t crc = 0xFFFFFFFF;
        while (true) {
            int ch = pStream->Get();
            if (ch == EOF) break;
            crc = Utils::ProcCRC_32(crc, static_cast<char>(ch));
        }
        return ~crc;
    }

}