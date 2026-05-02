#include "File.h"

#include "Debugger.h"
#include "Blob.h"

namespace GPC
{

    void File::DisplayHeader()
    {
        mp_Header->FileSize = (mp_Blob == nullptr ? 0 : mp_Blob->Size() );
        mp_Header->DestroyTag = m_IsDestroyed;
        GPC_INFO << " |= File(" << m_Index << ") | " << mp_Header->FileName;
        GPC_INFO << " |== Signature              : " << mp_Header->FileSignature;
        GPC_INFO << " |== Total File ByteSize    : " << mp_Header->FileSize;
        GPC_INFO << " |== Compressed File ByteSize : " << mp_Header->CompressedFileSize;
        GPC_INFO << " |== CRC                    : " << mp_Header->CRC;
        GPC_INFO << " |== Tagged Destroy         : " << (mp_Header->DestroyTag ? "True" : "False");
    }

    void File::DisplayContent()
    {
        if (IsLoaded() == false)
        {
            GPC_ERROR << "=|== File Content ======";
            GPC_ERROR << "/!\\ File not loaded !";
            GPC_ERROR << "=|======================";
            return;
        }
        ShowContent();
    }

    Blob* File::Init(uint64_t offsetInArchive, FileHeader* pHeader, Stream* pFile, uint32_t index, bool loadContent)
    {
        m_OffsetInArchive = offsetInArchive;
        mp_Header = pHeader;
        mp_Blob = nullptr;
        m_Path = pFile->GetPath().c_str();
        m_IsDestroyed = mp_Header->DestroyTag;
        m_Index = index;

        if (loadContent)
            LoadContent(pFile);

        m_IsLoaded = loadContent;

        return mp_Blob;

    }

    Blob* File::Create(FileHeader* pHeader, const char* path, uint32_t index)
    {
        m_OffsetInArchive = 0;
        m_Path = path;
        mp_Header = pHeader;
        mp_Blob = new Blob();
        m_Index = index;

        m_IsDestroyed = false;
        m_IsLoaded = true;

        return mp_Blob;
    }

    void File::LoadContent(Stream* pStream)
    {
        if (m_OffsetInArchive == 0) {
            GPC_INFO << " Trying to load content that you created !";
            return;
        }

        uint64_t size = mp_Header->CompressedFileSize;
        if (size == 0) {
            mp_Blob = new Blob();
            return;
        }

        auto* Buffer = new int8_t[mp_Header->CompressedFileSize];
        StreamUtils::RetrieveAll(
            pStream,
            Buffer,
            m_OffsetInArchive + sizeof(FileHeader),
            size
        );

        delete[] Buffer;
        m_IsLoaded = true;

    }

    void File::LoadContent(uint8_t* pData, uint64_t byteSize)
    {
        mp_Blob = new Blob(pData, byteSize);
    }

    void File::LoadFromFileContent(uint8_t* pData, uint64_t byteSize)
    {
        LoadContent(pData, byteSize);
    }

    uint64_t File::CopyTo(Stream* pIn, Stream* pOut, uint64_t offset)
    {
        GPC_INFO << " Copying from source : " << pIn->GetPath();

        uint64_t totalSize = mp_Header->CompressedFileSize + sizeof(FileHeader);
        uint64_t position = m_OffsetInArchive;
        GPC_INFO << " Copying " << totalSize << totalSize << "o at " << position << " : " << pOut->GetPath();

        auto* buffer = new int8_t[totalSize];
        StreamUtils::RetrieveAll(pIn, buffer, position, totalSize);
        StreamUtils::Edit(pOut, buffer, offset, totalSize);

        return totalSize;
    }

    void File::SaveData(Stream* pStream, uint64_t offset)
    {
        if (mp_Blob->Size() == 0) return;

        int8_t* Buffer = nullptr;
        mp_Header->CompressedFileSize = mp_Blob->Size();
        StreamUtils::Edit(pStream, Buffer, offset, mp_Blob->Size());

    }

    File::File()
    {
        m_OffsetInArchive = 0;
        m_Path = "";
        m_IsDestroyed = false;
        mp_Blob = nullptr;
        mp_Header = nullptr;
    }

    bool File::IsLoaded() const
    {
        return m_IsLoaded;
    }

    bool File::IsTaggedDestroyed() const
    {
        return m_IsDestroyed;
    }

    uint32_t File::Index() const
    {
        return m_Index;
    }

    File::Type File::CreateFilePtr(File** pFilePtr, uint32_t signature)
    {
        switch ((Type)signature)
        {
        case Type::BINARIES: {
                *pFilePtr = new BinaryFile();
                return Type::BINARIES;
        }
        case Type::TEXT: {
                *pFilePtr = new TextFile();
                return Type::TEXT;
        }
        default: return Type::UNKNOWN;
        }
    }

    void File::Destroy()
    {
        m_IsDestroyed = true;
    }

    void File::Restore()
    {
        m_IsDestroyed = false;
    }
}
