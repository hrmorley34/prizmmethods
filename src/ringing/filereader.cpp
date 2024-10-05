#include "../charset/charset.hpp"
#include "method.hpp"
#include "filereader.hpp"

#ifdef __sh__
#include <fxcg/file.h>

#include <fxcg/display.h>
#include "../utils.hpp"
#else
#include <iostream>
#include <fstream>
#endif

inline uint8_t ReadU8(const uint8_t *&ptr)
{
    return *ptr++;
}
inline uint16_t ReadU16(const uint8_t *&ptr)
{
    uint8_t lsb = *ptr++;
    uint8_t msb = *ptr++;
    return (uint16_t)msb << 8 | lsb;
}
inline uint32_t ReadU32(const uint8_t *&ptr)
{
    uint8_t lsb = *ptr++;
    uint8_t b2 = *ptr++;
    uint8_t b3 = *ptr++;
    uint8_t msb = *ptr++;
    return (uint32_t)msb << 24 | (uint32_t)b3 << 16 | (uint32_t)b2 << 8 | lsb;
}

#ifdef __sh__
inline int ReadFile(ringing::compat::FileHandle HANDLE, uint8_t *buf, int size, int readpos)
{
    return Bfile_ReadFile_OS(HANDLE, buf, size, readpos);
}
#else
inline int ReadFile(ringing::compat::FileHandle HANDLE, uint8_t *buf, int size, int readpos)
{
    if (readpos != -1)
        HANDLE->seekg(readpos);
    return HANDLE->read((char *)buf, size).gcount();
}
#endif

namespace ringing
{
    const char FILE_MAGIC_WORD[4] = {'C', 'C', 'M', 'L'};
    const int FILE_VERSION = 0x02;
    const int POINTERS_START = 0x08;

    bool FileReader::TryOpen(const compat::FileChar *const filename)
    {
#ifdef __sh__
        // Open file with mode 0 - read
        int handle = Bfile_OpenFile_OS(filename, 0, 0);
        if (handle < 0)
            return false;
#else
        std::ifstream *handle = new std::ifstream(filename, std::ios::in | std::ios::binary);
        if (handle == nullptr)
            return false;
#endif
        Close(); // ensure no previous file is still open
        filehandle = handle;
        if (!ReadHeader())
        {
            Close();
            return false;
        }
        return true;
    }

    bool FileReader::IsOpen()
    {
#ifdef __sh__
        return filehandle >= 0;
#else
        return filehandle != nullptr;
#endif
    }

    bool FileReader::ReadHeader()
    {
        uint8_t header[8];
        if (ReadFile(filehandle, header, sizeof(header), 0) != sizeof(header))
            return false;
        const uint8_t *header_ptr = header;
        for (int i = 0x00; i < 0x04; i++)
            if (ReadU8(header_ptr) != FILE_MAGIC_WORD[i])
                return false;
        if (ReadU8(header_ptr) != FILE_VERSION) // 0x04
            return false;
        stage = ReadU8(header_ptr);        // 0x05
        ReadU8(header_ptr);                // padding byte 0x06
        pointerdepth = ReadU8(header_ptr); // 0x07
        return true;
    }

    void FileReader::Close()
    {
        if (IsOpen())
        {
#ifdef __sh__
            Bfile_CloseFile_OS(filehandle);
#else
            filehandle->close();
            delete filehandle;
#endif
        }
        filehandle = compat::emptyFileHandle;
    }

    bool FileReader::ReadMethod(ringing::Method &method)
    {
        uint8_t data_header[2];
        if (ReadFile(filehandle, data_header, sizeof(data_header), -1) != sizeof(data_header))
            return false;
        const uint8_t *data_header_ptr = data_header;
        uint16_t data_length = ReadU16(data_header_ptr);
#ifdef __sh__
        uint8_t data[data_length];
#else
        // bad practice - only for CASIO compat
        uint8_t *data = (uint8_t *)alloca(data_length);
#endif
        if (ReadFile(filehandle, data, data_length, -1) != data_length)
            return false;

        method.stage = stage;

        const uint8_t *ptr = data;
        const uint8_t *endptr = data + data_length;
        if (ptr + 1 > endptr)
            return false;
        uint8_t methodname_length = ReadU8(ptr);
        if (ptr + methodname_length + 1 > endptr)
            return false;
        if (methodname_length + 1 > ringing::MAX_METHOD_TITLE_LENGTH)
            return false;
        for (int i = 0; i < methodname_length + 1; i++)
            method.title[i] = ReadU8(ptr);
        if (ptr + 2 > endptr)
            return false;
        method.leadlength = ReadU16(ptr);
        if (ptr + 2 * method.leadlength > endptr)
            return false;
        if (method.leadlength > ringing::MAX_PLACE_NOTATION_LENGTH)
            return false;
        for (int i = 0; i < method.leadlength; i++)
            method.pn[i] = ReadU16(ptr);
        if (ptr + 2 > endptr)
            return false;
        method.leadcount = ReadU16(ptr);
        if (ptr + 2 > endptr)
            return false;
        method.huntbells = ReadU16(ptr);
        // if (ptr != endptr) there was excess length - ignored
        return true;
    }

    bool FileReader::ReadMethodSummary(int *const pos, int *const stage, charset::MBChar *const title)
    {
        int start = Tell();
        if (pos != nullptr)
            *pos = start;

        uint8_t data_header[3];
        if (ReadFile(filehandle, data_header, sizeof(data_header), -1) != sizeof(data_header))
            return false;

        const uint8_t *data_header_ptr = data_header;
        uint16_t data_length = ReadU16(data_header_ptr);
        if (data_length <= 0)
            return false;
        if (data_length >= 1024)
        {
#ifdef __sh__
            PrintXY(1, 7, "  HUGE READ!", TEXT_MODE_NORMAL, TEXT_COLOR_RED);
            DebugFreeze();
#endif
            return false;
        }

        if (stage != nullptr)
            *stage = this->stage;
        if (title != nullptr)
        {
            uint8_t methodname_length = ReadU8(data_header_ptr);
            if (data_length <= 1 + (methodname_length + 1))
                return false;
            if (methodname_length + 1 > ringing::MAX_METHOD_TITLE_LENGTH)
                return false;
            if (ReadFile(filehandle, (uint8_t *)title, methodname_length + 1, -1) != methodname_length + 1)
                return false;
            if (title[methodname_length] != 0)
                return false;
        }

        Seek(start + sizeof(data_length) + data_length);

        return true;
    }

    bool FileReader::Search(const charset::NonMBChar *const searchstring, int *const pos)
    {
        if (pointerdepth < 0)
            return false;
        if (searchstring == nullptr)
            return false;

        int pointerindex = charset::GetSearchPointerIndex(searchstring, pointerdepth);

        uint8_t pointer_raw[4];
        const int pointer_pos = POINTERS_START + sizeof(pointer_raw) * pointerindex;
        if (ReadFile(filehandle, pointer_raw, sizeof(pointer_raw), pointer_pos) != sizeof(pointer_raw))
            return false;
        const uint8_t *pointer_raw_ptr = pointer_raw;
        int pointer_dest = ReadU32(pointer_raw_ptr);
        Seek(pointer_dest);

        charset::MBChar title[MAX_METHOD_TITLE_LENGTH];
        do
        {
            if (EndOfFile())
            {
                pointer_dest = Size();
                break;
            }
            if (!ReadMethodSummary(&pointer_dest, nullptr, title))
            {
#ifdef __sh__
                PrintXY(1, 6, "  Failed read.", TEXT_MODE_NORMAL, TEXT_COLOR_RED);
                DebugFreeze();
#endif
                return false;
            }
        } while (charset::CompareSearch(searchstring, title) == charset::CompareResult::BeforeKey);

        Seek(pointer_dest);
        if (pos != nullptr)
            *pos = pointer_dest;
        return true;
    }

#ifdef __sh__
    int FileReader::Tell() { return Bfile_TellFile_OS(filehandle); }
    void FileReader::Seek(int pos) { Bfile_SeekFile_OS(filehandle, pos); }
    int FileReader::Size() { return Bfile_GetFileSize_OS(filehandle); }
#else
    int FileReader::Tell() { return filehandle->tellg(); }
    void FileReader::Seek(int pos) { filehandle->seekg(pos); }
    int FileReader::Size()
    {
        int cpos = Tell();
        filehandle->seekg(0, std::ios::end);
        int size = Tell();
        Seek(cpos);
        return size;
    }
#endif
}
