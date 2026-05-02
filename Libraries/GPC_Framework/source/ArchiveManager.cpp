//
// Created by hugoc on 06/03/2026.
//
#include "ArchiveManager.h"
#include <filesystem>
#include "Stream.h"

namespace GPC
{
	const std::map<std::string, File::Type> ArchiveManager::ExtentionAuthorized = std::map<std::string, File::Type>(
		{
			{ std::string("txt"), File::Type::TEXT },
			{ std::string("PNG"), File::Type::PNG },
			{ std::string("png"), File::Type::PNG },
			{ std::string("bin"), File::Type::BINARIES },
		}
	);

	void ArchiveManager::UpdateFileHeader(uint32_t index, bool updateCRC)
	{
		mp_FileHeaders[index]->FileSignature = static_cast<uint32_t>(mp_Files[index]->GetType());
		mp_FileHeaders[index]->DestroyTag = mp_Files[index]->IsTaggedDestroyed();
		if (mp_Files[index]->IsLoaded()) {
			if (updateCRC) mp_FileHeaders[index]->CRC = Utils::CRC_32(mp_Contents[index]);
			mp_FileHeaders[index]->FileSize = mp_Contents[index]->Size();
		}
	}

	void ArchiveManager::UpdateArchiveHeader()
	{
		m_ArchiveHeader.ArchiveFileCount = GetFileCount();
		m_ArchiveHeader.ArchiveSize = sizeof(FileHeader) * GetFileCount();
		for (uint32_t i = 0; i < GetFileCount(); ++i) {
			m_ArchiveHeader.ArchiveSize += mp_FileHeaders[i]->FileSize;
		}
	}

	bool ArchiveManager::CheckFileCRC(uint32_t index)
	{
		if(mp_Files[index]->IsLoaded())
			return Utils::CRC_32(mp_Contents[index]) == mp_FileHeaders[index]->CRC;
		GPC_INFO << "Check CRC of non-loaded file";
		return false;
	}

	void ArchiveManager::LoadArchHeader(Stream* pStream)
	{
		// Loading ArchiveHeader
		m_ArchiveHeader = StreamUtils::Retrieve<ArchiveHeader>(pStream, 0);
		if (m_ArchiveHeader.Signature != ARCHIVE_SGNATURE) {
			 GPC_ERROR << "Wrong Signature !";
			return;
		}

		if (m_ArchiveHeader.ArchiveFileCount == 0) {
			GPC_INFO << " Empty Archive !";
			return;
		}

		mp_Contents.resize(GetFileCount());
		mp_FileHeaders.resize(GetFileCount());
		mp_Files.resize(GetFileCount());
	}

	void ArchiveManager::LoadFile(Stream* pStream, uint32_t index, uint64_t offset, bool doLoadContent)
	{
		mp_FileHeaders[index] = new FileHeader();
		StreamUtils::RetrieveAll(pStream, mp_FileHeaders[index], offset);

		if (File::CreateFilePtr(&mp_Files[index], mp_FileHeaders[index]->FileSignature) == File::Type::UNKNOWN) {
			GPC_INFO << "Unknown File type";
		}
		if (mp_Files[index] == nullptr) {
			GPC_ERROR << "Unable to Create File !";
		}

		mp_Contents[index] = mp_Files[index]->Init(offset, mp_FileHeaders[index], pStream, index, doLoadContent);
		if (doLoadContent == false) return;

		if (mp_Contents[index] == nullptr) {
			GPC_ERROR << "Unable to Create Content !";
		}

		if (CheckFileCRC(index) == false) {
			GPC_ERROR << " File was corrupted !";
		}
	}

	void ArchiveManager::DisplayArchiveHeader() const
	{
		GPC_INFO << "Archive";
		GPC_INFO << " |= Signature         : " << m_ArchiveHeader.Signature;
		GPC_INFO << " |= Archive ByteSize  : " << m_ArchiveHeader.ArchiveSize;
		GPC_INFO << " |= FileCount         : " << m_ArchiveHeader.ArchiveFileCount;
		GPC_INFO << " |= CRC               : " << m_ArchiveHeader.CRC;
	}

	void ArchiveManager::DisplayFile(uint32_t index) const
	{
		mp_Files[index]->DisplayHeader();
		mp_Files[index]->DisplayContent();
	}

	ArchiveManager::ArchiveManager(const char* path)
	{
		m_Path = path;
		m_ArchiveHeader = ArchiveHeader();
		m_IsInit = false;
		m_ArchiveExist = false;
	}

	ArchiveManager::ArchiveManager()
	{
		m_Path = "";
		m_ArchiveHeader = ArchiveHeader();
		m_IsInit = false;
		m_ArchiveExist = false;
	}

	uint32_t ArchiveManager::GetFileCount() const
	{
		return m_ArchiveHeader.ArchiveFileCount;
	}

	void ArchiveManager::CreateArchive()
	{
		m_ArchiveExist = false;
		m_ArchiveHeader.Signature = ARCHIVE_SGNATURE;
		m_ArchiveHeader.ArchiveSize = 0;
		m_ArchiveHeader.ArchiveFileCount = 0;
	}

	void ArchiveManager::OpenArchive()
	{
		m_ArchiveExist = Utils::FileExist(m_Path);
		if (m_ArchiveExist == false) {
			GPC_INFO << " File not found : Creating Archive(" << m_Path << ")";
			CreateArchive();
			return;
		}

		ReadStream stream(m_Path);
		LoadArchHeader(&stream);

		uint32_t crc = StreamUtils::CRC_32_From(&stream, sizeof(ArchiveHeader));
		if (crc != m_ArchiveHeader.CRC) {
			GPC_ERROR << "Archive corrupted !";
			return;
		}

		uint64_t offset = sizeof(ArchiveHeader);
		for (uint32_t i = 0; i < GetFileCount(); ++i) {
			mp_Contents[i] = nullptr;
			mp_FileHeaders[i] = nullptr;
			mp_Files[i] = nullptr;
			LoadFile(&stream, i, offset, false);

			offset += sizeof(FileHeader) + mp_FileHeaders[i]->CompressedFileSize;
		}

		stream.Close();
	}

	File* ArchiveManager::AddFile(const char* path)
	{
		if (std::filesystem::exists(path) == false) {
			GPC_INFO << "File not found : ignored";
			return nullptr;
		}
		std::string fileName = Utils::GetFileName(path);
		std::string extention = Utils::GetFileExtension(path);


		if (extention.c_str() == nullptr || ExtentionAuthorized.contains(extention) == false) {
			GPC_INFO << "Extention not supported : ignored";
			return nullptr;
		}
		GPC_INFO << "Loading file " << path;
		File::Type type = ExtentionAuthorized.at(extention);
		File* pFile = AddFile(type, fileName.c_str());
		if (pFile == nullptr) return nullptr;

		ReadStream stream(path);

		uint8_t* buffer = nullptr;
		uint64_t size = stream.ReadAll(&buffer);
		mp_FileHeaders[GetFileCount() - 1]->FileSize = size;

		pFile->LoadFromFileContent(buffer, size);
		stream.Close();

		m_ArchiveHeader.ArchiveSize += size;
		return pFile;
	}

	std::vector<File*> ArchiveManager::AddFiles(const char* path)
	{
		GPC_INFO << "Loading files from " << path;
		std::vector<File*> list;

		for (const auto& entry : std::filesystem::directory_iterator(path)) {
			File* pFile = AddFile(entry.path().string().c_str());
			if(pFile != nullptr)
				list.push_back(pFile);
		}
		return list;
	}

	File* ArchiveManager::AddFile(File::Type type, const char name[32])
	{
		GPC_INFO << "Creating file " << name << " as " << (uint32_t)type ;

		for (uint32_t i = 0; i < GetFileCount(); ++i) {
			if (Utils::IsSame(name, mp_FileHeaders[i]->FileName)) {
				GPC_INFO << "File already exist : ignored";
				return nullptr;
			}
		}

		uint32_t index = static_cast<uint32_t>(mp_Files.size());
		uint32_t NameSize = static_cast<uint32_t>(strlen(name));
		if (NameSize >= 32) {
			NameSize = 31;
		}

		FileHeader* fileHeader = new FileHeader();
		fileHeader->FileSignature = static_cast<uint32_t>(type);
		fileHeader->FileSize = 0;
		fileHeader->CompressedFileSize = 0;
		fileHeader->CRC = 0;
		memset(fileHeader->FileName, 0, 32);
		memcpy(fileHeader->FileName, name, NameSize);
		fileHeader->FileName[NameSize] = '\0';

		mp_FileHeaders.push_back(fileHeader);
		mp_Files.push_back(nullptr);

		if (File::CreateFilePtr(&mp_Files[index], (uint32_t)type) == File::Type::UNKNOWN) {
			GPC_ERROR << "File type unknown.";
			return nullptr;
		}
		Blob* pBlob = mp_Files[index]->Create(fileHeader, m_Path, GetFileCount());
		if (pBlob == nullptr) {
			GPC_ERROR << "Failed to create blob.";
			return nullptr;
		}
		mp_Contents.push_back(pBlob);

		m_ArchiveHeader.ArchiveFileCount++;
		return mp_Files[index];
	}

	File* ArchiveManager::GetFile(uint32_t index)
	{
		if (index < 0 || index >= GetFileCount()) {
			GPC_ERROR << "File index out of bounds";
			return nullptr;
		}

		if (mp_Files[index]->IsLoaded() == false) {
			ReadStream file(m_Path);
			mp_Files[index]->LoadContent(&file);
			mp_Contents[index] = mp_Files[index]->mp_Blob;
			file.Close();
		}
		return mp_Files[index];
	}

	File* ArchiveManager::GetFile(const char* name)
	{
		for (uint32_t i = 0; i < GetFileCount(); ++i) {
			if (Utils::IsSame(mp_FileHeaders[i]->FileName, name)) {
				return GetFile(i);
			}
		}
		return nullptr;
	}

	void ArchiveManager::CloseMemory(bool purge)
	{
		GPC_INFO << "Starting to Save Archive";
		GPC_INFO << " Saving " << GetFileCount() << " Files.";

		uint32_t pathLength = static_cast<uint32_t>(strlen(m_Path));
		char* TempPath = new char[pathLength + 5];
		const char* Suffixe = "-Temp";
		for (uint32_t i = 0; i < pathLength; ++i) {
			TempPath[i] = Suffixe[i];
		}
		for (uint32_t i = 0; i < 5; i++)
		{
			TempPath[pathLength + i] = Suffixe[i];
		}

		OverwriteStream Stream(TempPath);
		GPC::Stream* pInput = nullptr;
		if(m_ArchiveExist)
			pInput = new ReadStream(m_Path);

		uint32_t finalFileCount = 0;
		uint64_t offset = sizeof(ArchiveHeader);
		for (uint32_t i = 0; i < GetFileCount(); ++i) {
			if (purge && mp_Files[i]->IsTaggedDestroyed()) {
				GPC_INFO << "File(" << i << ") : " << mp_FileHeaders[i]->FileName << " Has been purged";
				continue;
			}
			GPC_INFO << "Saving File(" << i << ")";
			finalFileCount++;

			if (mp_Files[i]->IsLoaded() == false) {
				GPC_INFO << " File wasnt loaded";
				if(m_ArchiveExist)
					GPC_INFO << " Copying from source";
					uint64_t byteCopied = mp_Files[i]->CopyTo(pInput, &Stream, offset);
					offset += byteCopied;
					GPC_INFO << "File " << i << " | Size: " << byteCopied << "o => COPIED !";
				continue;
			}

			GPC_INFO << " File was loaded";

			UpdateFileHeader(i, true);
			GPC_INFO << " Updating Header...";

			GPC_INFO << " Saving Content...";
			mp_Files[i]->SaveData(&Stream, offset + sizeof(FileHeader));

			GPC_INFO << " Updated Header : ";
			mp_Files[i]->DisplayHeader();
			GPC_INFO << " Saving Header...";
			StreamUtils::Edit(&Stream, mp_FileHeaders[i], offset);

			GPC_INFO << "File " << i << " | Size: " << mp_FileHeaders[i]->CompressedFileSize << "o => SAVED !";
			offset += sizeof(FileHeader) + mp_FileHeaders[i]->CompressedFileSize;
		}

		if (m_ArchiveExist)
			pInput->Close();
		Stream.Close();

		ReadWriteStream ArchiveModif(TempPath);

		GPC_INFO << " Updating Archive Header...";
		UpdateArchiveHeader();
		m_ArchiveHeader.ArchiveFileCount = finalFileCount; // Update According to Purged files
		m_ArchiveHeader.CRC = StreamUtils::CRC_32_From(&ArchiveModif, sizeof(ArchiveHeader));
		GPC_INFO << " Updated Archive Header...";
		DisplayArchiveHeader();
		GPC_INFO << " Saving Archive Header...";
		StreamUtils::Edit(&ArchiveModif, &m_ArchiveHeader, 0);

		ArchiveModif.Close();

		for (uint32_t i = 0; i < GetFileCount(); ++i) {
			delete mp_FileHeaders[i];
			delete mp_Contents[i];
			delete mp_Files[i];
		}

		mp_FileHeaders.clear();
		mp_Contents.clear();
		mp_Files.clear();

		GPC_INFO << " Archive saved on -Temp";

		if (m_ArchiveExist && remove(m_Path) != 0) {
			GPC_ERROR << "Failed to remove old file";
		}
		if (rename(TempPath, m_Path) != 0) {
			GPC_ERROR << "Failed to rename file" << TempPath << " with name " << m_Path;
		}
		GPC_INFO << " Archive saved !";
	}

	void ArchiveManager::Display()
	{
		DisplayArchiveHeader();
		for (uint32_t i = 0; i < GetFileCount(); ++i) {
			mp_Files[i]->DisplayHeader();
			mp_Files[i]->DisplayContent();
		}
	}

	uint32_t Utils::ProcCRC_32(uint32_t prevCRC, char data)
	{
		for (size_t j = 0; j < 8; j++) {
			uint32_t b = (data ^ prevCRC) & 1;
			prevCRC >>= 1;
			if (b) prevCRC = prevCRC ^ 0xEDB88320;
			prevCRC >>= 1;
		}
		return prevCRC;
	}


	uint32_t Utils::CRC_32(const uint8_t* pData, uint64_t length)
	{
		uint32_t crc = 0xFFFFFFFF;
		for (uint64_t i = 0; i < length; i++) {
			char ch = pData[i];
			crc = ProcCRC_32(crc, ch);
		}
		return ~crc;
	}

	uint32_t Utils::CRC_32(Blob* pBlob)
	{
		if (pBlob->Size() == 0) return 0;
		return CRC_32(pBlob->Ptr(), pBlob->Size());
	}

	bool Utils::FileExist(const char* path)
	{
		return std::filesystem::exists(path);
	}

	std::string Utils::GetFileName(const std::string& filePath)
	{
		const size_t lastSlashPos = filePath.find_last_of("/\\");
		const size_t dotPos = filePath.find_last_of('.');

		std::string fileName = (lastSlashPos == std::string::npos) ? filePath : filePath.substr(lastSlashPos + 1);

		if (dotPos != std::string::npos)
		{
			fileName = fileName.substr(0, dotPos);
		}

		return fileName;
	}

	std::string Utils::GetFileExtension(const std::string& filePath)
	{
		const size_t dotPos = filePath.find_last_of('.');

		if (dotPos == std::string::npos || dotPos == filePath.length() - 1)
		{
			return "";
		}

		return filePath.substr(dotPos + 1);
	}

	void Utils::CheckFileName(std::string& path)
	{
		if (path.find_last_of("/\\") != std::string::npos)
		{
			path = GetFileName(path);
		}
	}

	bool Utils::IsSame(const char* str1, const char* str2)
	{
		uint32_t size1 = static_cast<uint32_t>(strlen(str1));
		uint32_t size2 = static_cast<uint32_t>(strlen(str2));
		if (size1 != size2) return false;

		for (uint32_t i = 0; i < (size1 > size2 ? size1 : size2); ++i)
		{
			if (str1[i] != str2[i]) return false;
		}

		return true;
	}
}
