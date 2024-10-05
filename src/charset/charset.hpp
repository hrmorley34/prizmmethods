#ifndef CHARSET_HPP
#define CHARSET_HPP

namespace charset
{
    typedef char MBChar;
    typedef char NonMBChar;

    NonMBChar ReadSearchChar(const MBChar *&c);

    struct CharCount
    {
        int bytes;
        int characters;

        inline CharCount operator+=(const CharCount &b)
        {
            this->bytes += b.bytes;
            this->characters += b.characters;
            return *this;
        }

        inline friend CharCount operator+(CharCount lhs, const CharCount &rhs)
        {
            lhs += rhs;
            return lhs;
        }

        inline CharCount operator++()
        {
            this->bytes++;
            this->characters++;
            return *this;
        }

        inline CharCount operator--()
        {
            this->bytes--;
            if (this->bytes < 0)
                this->bytes = 0;
            this->characters--;
            if (this->characters < 0)
                this->characters = 0;
            return *this;
        }
    };

    CharCount CopyString(const MBChar *src, MBChar *dest, int max_char_count, int max_byte_count, bool force_null);

    enum CompareResult : char
    {
        Err = -2,
        BeforeKey = -1,
        Contained = 0,
        AfterKey = 1,
    };

    inline CompareResult CompareSearch(const NonMBChar *searchkey, const MBChar *text)
    {
        if (searchkey == nullptr || text == nullptr)
            return Err;
        while (true)
        {
            NonMBChar s = ReadSearchChar(searchkey), t = ReadSearchChar(text);
            if (s == '\0')
                return Contained;
            else if (t == '\0' || t < s)
                return BeforeKey;
            else if (t > s)
                return AfterKey;
        }
    }

    int GetSearchPointerIndex(const NonMBChar *searchstring, int pointerdepth);
}

#endif
