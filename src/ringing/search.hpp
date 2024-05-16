#ifndef RINGING_SEARCH_HPP
#define RINGING_SEARCH_HPP

namespace ringing
{
// `min <= i <= max`
#define Between(min, i, max) ((min) <= (i) && (i) <= (max))
    inline bool SkipTitleChar(const char c)
    {
        return !(c == '\0' || Between(0x20, c, 0x7e));
    }
    inline char TransformTitleChar(const char *&c)
    {
        // while (SkipTitleChar(*c))
        //     c++; // skip special characters and control characters
        if (*c == '\0')
            return '\0';
        if (Between('a', *c, 'z'))
            return *c++ - 'a' + 'A'; // to upper case
        if (Between('A', *c, 'Z') || Between('0', *c, '9'))
            return *c++;
        c++;
        return ' ';
    }

    inline bool CompareTitles(const char *searchstr, const char *text)
    {
        if (searchstr == nullptr || text == nullptr)
            return false;
        while (true)
        {
            char s = TransformTitleChar(searchstr), t = TransformTitleChar(text);
            if (s == '\0' || t == '\0')
                break;
            if (s != t)
                return false;
        }
        return *searchstr == '\0'; // check for end of search string
    }

    inline bool CompareTitlesLessThan(const char *text, const char *searchstr)
    {
        if (searchstr == nullptr || text == nullptr)
            return false;
        while (*searchstr != '\0' && *text != '\0') // end of string
        {
            // char us = *searchstr, ut = *text;
            int s = TransformTitleChar(searchstr), t = TransformTitleChar(text);
            // Compare transformed form
            if (t < s)
                return true;
            else if (t > s)
                return false;
            // // Compare untransformed form
            // else if (ut<us)
            //     return true;
            // else if (ut>us)
            //     return false;
        }
        return *searchstr != '\0'; // ensure search string doesn't match to be g.t.
    }

    const int TitleFPointerOffset = 2;
    const int TitleFPointerChars = 26 + 2;

    inline int TransformTitleFPointerChar(const char *&c)
    {
        // while (SkipTitleChar(*c))
        //     c++;
        if (Between('0', *c, '9'))
        {
            c++;
            return -1;
        }
        if (Between('A', *c, 'Z'))
            return (*c++ - 'A');
        if (Between('a', *c, 'z'))
            return (*c++ - 'a');
        if (*c != '\0')
            c++;
        return -2;
    }
#undef Between
}

#endif
