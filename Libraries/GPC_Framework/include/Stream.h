//
// Created by hugoc on 03/03/2026.
//

#pragma once

#include <string>
#include "Debugger.h"
#include <type_traits>

#define AS_FLAG(T) \
inline constexpr T operator | (T a, T b) { \
return static_cast<T>( \
static_cast<std::underlying_type_t<T>>(a) | \
static_cast<std::underlying_type_t<T>>(b) \
); \
}

namespace GPC
{
    class Stream
    {
        public:
        enum class FileOpenType : uint32_t
        {
            DELETE	= 1,
            READ	= 2,
            WRITE	= 4
        };

        Stream();
        Stream(std::string_view path, uint32_t openType);
    	virtual ~Stream();
        void OpenAs(const char* Mode);
        void Close();

        const std::string& GetPath();
        [[nodiscard]] uint64_t GetSize() const;
        [[nodiscard]] bool IsOpenType(FileOpenType openType) const;

        void PlaceCursor(uint64_t offset) const;
    	int Get();


        void Write(const void* pData, uint64_t byteSize);
        void WriteAt(const void* pData, uint64_t offset, uint64_t byteSize);
        void WriteEnd(const void* pData, uint64_t byteSize);

        uint64_t ReadAll(uint8_t** pData);
        void ReadAt(void* pData, uint64_t offset, uint64_t byteSize);

    private:

        void CheckFileSize();
        void SeekEnd();
        void SeekStart();

        // Variables
        std::string m_Path;
        FILE* mp_File{};
        uint64_t m_FileByteSize;
        uint32_t m_OpenType;
        bool m_IsOpen{};
    };

	AS_FLAG(Stream::FileOpenType);

	//#########################################
	//              STREAM TYPE
	//#########################################

	class ReadStream : public Stream {
	public:
		ReadStream(std::string_view path);

		void Write(const void* pData, uint64_t byteSize) = delete;
		void WriteAt(const void* pData, uint64_t offset, uint64_t byteSize) = delete;
		void WriteEnd(const void* pData, uint64_t byteSize) = delete;
	};

	class WriteStream : public Stream {
	public:
		WriteStream(std::string_view path);

		uint64_t ReadAll(char** pData) = delete;
		void ReadAt(void* pData, uint64_t offset, uint64_t byteSize) = delete;
	};

	class OverwriteStream : public Stream {
	public:
		OverwriteStream(std::string_view path);

		uint64_t ReadAll(char** pData) = delete;
		void ReadAt(void* pData, uint64_t offset, uint64_t byteSize) = delete;
	};

	class ReadWriteStream : public Stream {
	public:
		ReadWriteStream(std::string_view path);
	};

	class ReadOverwriteStream : public Stream {
	public:
		ReadOverwriteStream(std::string_view path);
	};

	//#########################################
	//              STREAM UTILS
	//#########################################

    class StreamUtils {
    public:
        template<typename T>
        static void Fill(std::string_view path, T* pData, uint64_t count = 1);
        template<typename T>
        static void Fill(Stream* pFile, T* pData, uint64_t count = 1);

        template<typename T>
        static T Retrieve(const char* path, uint64_t offset);
        template<typename T>
        static T Retrieve(Stream* pFile, uint64_t offset);

        template<typename T>
        static void RetrieveAll(const char* path, T* pData, uint64_t offset, uint64_t count = 1);
        template<typename T>
        static void RetrieveAll(Stream* pFile, T* pData, uint64_t offset, uint64_t count = 1);

        template<typename T>
        static void Edit(const char* path, T* pData, uint64_t offset, uint64_t count = 1);
        template<typename T>
        static void Edit(Stream* pFile, T* pData, uint64_t offset, uint64_t count = 1);

        template<typename T>
        static void Push(const char* path, T* pData, uint64_t count = 1);
        template<typename T>
        static void Push(Stream* pFile, T* pData, uint64_t count = 1);

        static uint32_t CRC_32_From(Stream* pStream, uint64_t from);
    };

    //#########################################
    //              TEMPLATE
    //#########################################

    template<typename T>
	inline void StreamUtils::Fill(std::string_view path, T* pData, uint64_t count)
	{
		OverwriteStream file(path);

		StreamUtils::Fill(&file, pData, count);

		file.Close();
	}

	template<typename T>
	inline void StreamUtils::Fill(Stream* pFile, T* pData, uint64_t count)
	{
		if (pFile->IsOpenType(Stream::FileOpenType::WRITE)) {

			pFile->Write(pData, count * sizeof(T));
		}
		else {
			GPC_ERROR << "File was not open with mode WRITE : ignored";
		}
	}

	template<typename T>
	inline T StreamUtils::Retrieve(const char* path, uint64_t offset)
	{
		ReadStream file(path);

    	T data = StreamUtils::Retrieve<T>(&file, offset);

		file.Close();

		return data;
	}

	template<typename T>
	inline T StreamUtils::Retrieve(Stream* pFile, uint64_t offset)
	{
		T obj;
		if (pFile->IsOpenType(Stream::FileOpenType::READ)) {
			pFile->ReadAt(&obj, offset, sizeof(T));
		}
		return obj;
	}

	template<typename T>
	inline void StreamUtils::RetrieveAll(const char* path, T* pData, uint64_t offset, uint64_t count)
	{
		ReadStream file(path);

		StreamUtils::RetrieveAll(&file, pData, offset, count);

		file.Close();
	}

	template<typename T>
	inline void StreamUtils::RetrieveAll(Stream* pFile, T* pData, uint64_t offset, uint64_t count)
	{
		if (count == 0) return;
		if (pFile->IsOpenType(Stream::FileOpenType::READ)) {
			pFile->ReadAt(pData, offset, sizeof(T) * count);
		}
		else {
			GPC_ERROR << "File was not open with mode READ : ignored";
		}
	}

	template<typename T>
	inline void StreamUtils::Edit(const char* path, T* pData, uint64_t offset, uint64_t count)
	{
		WriteStream file(path);

		StreamUtils::Edit(&file, pData, offset, count);

		file.Close();
	}

	template<typename T>
	inline void StreamUtils::Edit(Stream* pFile, T* pData, uint64_t offset, uint64_t count)
	{
		if (pFile->IsOpenType(Stream::FileOpenType::WRITE)) {
			pFile->WriteAt(pData, offset, sizeof(T) * count);
		}
		else {
			GPC_ERROR << "File was not open with mode WRITE : ignored";
		}
	}

	template<typename T>
	inline void StreamUtils::Push(const char* path, T* pData, uint64_t count)
	{
		WriteStream file(path);

		StreamUtils::Push(&file, pData, count);

		file.Close();
	}

	template<typename T>
	inline void StreamUtils::Push(Stream* pFile, T* pData, uint64_t count)
	{
		if (pFile->IsOpenType(Stream::FileOpenType::WRITE)) {
			pFile->WriteEnd(pData, sizeof(T) * count);
		}
		else {
			GPC_ERROR << "File was not open with mode WRITE : ignored";
		}
	}
}