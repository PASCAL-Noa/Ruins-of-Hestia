//
// Created by hugoc on 04/03/2026.
//

#pragma once

#define BINARY_SIGNATURE 0x00000001U
#define TEXT_SIGNATURE   0x00000002U
#define PNG_SIGNATURE	 0x89504E47U

#include "Blob.h"
#include "Stream.h"

namespace GPC
{
    class BinaryFile;

    struct FileHeader
    {
        char FileName[32]			= "";
        uint64_t FileSize			= 0;
        uint64_t CompressedFileSize	= 0;
        uint32_t FileSignature		= 0;
        uint32_t CRC			    = 0;
        bool DestroyTag			    = false;
    };

    class File
    {
    public:
        virtual ~File() = default;

        enum class Type : uint32_t
        {
            PNG = PNG_SIGNATURE,
            BINARIES = BINARY_SIGNATURE,
            TEXT = TEXT_SIGNATURE,
            UNKNOWN = 0x00000000U,
        };

        File();
        virtual Type GetType() = 0;

        // void SetKey(ArchiveKey* pKey);

        [[nodiscard]] bool IsLoaded() const;
        [[nodiscard]] bool IsTaggedDestroyed() const;
        [[nodiscard]] uint32_t Index() const;
        //[[nodiscard]] static bool HasKey();

        static File::Type CreateFilePtr(File** pFilePtr, uint32_t signature);

        template<typename T>
        void Add(const T* pData, uint64_t length);

        void Destroy();
        void Restore();

        virtual void ShowContent() = 0;



    protected:
        const char* m_Path;
        uint64_t m_OffsetInArchive;
        uint64_t m_Index = 0;
        FileHeader* mp_Header;
        Blob* mp_Blob;
        //ArchiveKey* mp_Key;

        bool m_IsDestroyed = false;
        bool m_IsLoaded = false;

        void DisplayHeader();
        void DisplayContent();

    private:
        friend class ArchiveManager;

        Blob* Init(uint64_t offsetInArchive, FileHeader* pHeader, Stream* pFile, uint32_t index, bool loadContent = true);
        Blob* Create(FileHeader* pHeader, const char* path, uint32_t index);

        void LoadContent(Stream* pStream);
        void LoadContent(uint8_t* pData, uint64_t byteSize);
        void LoadFromFileContent(uint8_t* pData, uint64_t byteSize);

        uint64_t CopyTo(Stream* pIn, Stream* pOut, uint64_t offset);
        void SaveData(Stream* pStream, uint64_t offset);
    };

    //#########################################
    //              FILE TYPE
    //#########################################

    class TextFile : public File {
    public:
        TextFile() = default;
        Type GetType() override { return Type::TEXT; };
        // TODO : implement those functions
        // void Add(const char* pData, uint64_t length);

        void ShowContent() override {}
    };

    class BinaryFile : public File {
    public:
        BinaryFile() = default;
        Type GetType() override { return Type::BINARIES; };

        // TODO : implement this functions
        void ShowContent() override { }
    };

    //#########################################
    //              TEMPLATES
    //#########################################

    template <typename T>
    void File::Add(const T* pData, uint64_t length)
    {
        mp_Blob->Add(pData, length);
    }
}
