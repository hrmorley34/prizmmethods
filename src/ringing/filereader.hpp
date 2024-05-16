#include "method.hpp"
#include "search.hpp"

#ifdef __sh__
#else
#include <fstream>
#endif

#ifndef RINGING_FILEREADER_HPP
#define RINGING_FILEREADER_HPP

namespace ringing
{
    namespace compat
    {
#ifdef __sh__
        typedef int FileHandle;
        typedef unsigned short FileChar;
        const FileHandle emptyFileHandle = -1;
#else
        typedef std::ifstream *FileHandle;
        typedef char FileChar;
        const FileHandle emptyFileHandle = nullptr;
#endif
    }

    class FileReader
    {
    private:
        compat::FileHandle filehandle;
        int stage;

        int pointerdepth;

    public:
        FileReader(compat::FileHandle filehandle = compat::emptyFileHandle) : filehandle(filehandle) {}
#ifndef __sh__
        ~FileReader() { Close(); }
#endif
        bool TryOpen(const compat::FileChar *filename);
        void Close();
        bool IsOpen();
        bool ReadHeader();

        bool ReadMethod(Method &method);
        bool ReadMethodSummary(int *pos, int *stage, char *title);

        bool Search(const char *searchstring, int *pos);

        int Tell();
        void Seek(int pos);
        int Size();
        bool EndOfFile() { return Tell() == Size(); }
    };
}

#endif
