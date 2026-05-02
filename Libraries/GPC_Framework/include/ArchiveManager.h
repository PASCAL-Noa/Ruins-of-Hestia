//
// Created by hugoc on 06/03/2026.
//

#pragma once
#include <map>
#include <string>
#include "File.h"

#define ARCHIVE_SGNATURE 0x41524348

namespace GPC
{

    struct ArchiveHeader
    {
        uint32_t Signature;
        uint32_t Version;
        uint32_t ArchiveFileCount;
        uint32_t CRC;
        uint64_t ArchiveSize;
    };

    class ArchiveManager
    {
    public:
        static const std::map<std::string, File::Type> ExtentionAuthorized;
    private:
        bool m_IsInit;
        const char* m_Path;
        bool m_ArchiveExist;

        ArchiveHeader m_ArchiveHeader;

        std::vector<Blob*> mp_Contents;
        std::vector<FileHeader*> mp_FileHeaders;
        std::vector<File*> mp_Files;

        void UpdateFileHeader(uint32_t index, bool updateCRC = false);
        void UpdateArchiveHeader();


        // === Loading ===
        void LoadArchHeader(Stream* pStream);
        void LoadFile(Stream* pStream, uint32_t index, uint64_t offset, bool doLoadContent = true);

        friend class File;
    public:
        ArchiveManager(const char* path);
        ArchiveManager();

        // === DISPLAY ===
        void DisplayArchiveHeader() const;
        void DisplayFile(uint32_t index) const;

        bool CheckFileCRC(uint32_t index);
        uint32_t GetFileCount() const;

        void CreateArchive();
        void OpenArchive();

        File* AddFile(const char* path);
        std::vector<File*> AddFiles(const char* path);
        File* AddFile(File::Type type, const char name[32]);

        File* GetFile(uint32_t index);
        File* GetFile(const char* name);

        void CloseMemory(bool purge = false);

        void Display();
    };

    class Utils
    {
        public:
        static uint32_t ProcCRC_32(uint32_t prevCRC, char data);
        static uint32_t CRC_32(const uint8_t* pData, uint64_t length);
        static uint32_t CRC_32(Blob* pBlob);
        static bool FileExist(const char* path);

        static std::string GetFileExtension(const std::string& filePath);
        static std::string GetFileName(const std::string& filePath);
        static void CheckFileName(std::string& path);
        static bool IsSame(const char* str1, const char* str2);
    };
}