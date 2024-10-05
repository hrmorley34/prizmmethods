#include <fxcg/system.h>
#include "charset.hpp"

namespace charset
{
#include "gen.hpp"

    CharCount CopyString(const MBChar *src, MBChar *dest, int max_char_count, int max_byte_count, bool force_null)
    {
        CharCount count = {0, 0};
        MBChar c;
        while (count.characters < max_char_count && count.bytes < max_byte_count && *src != 0)
        {
            *dest++ = c = *src++;
            ++count;
            if (*src != 0 && MB_IsLead(c))
            {
                if (count.bytes >= max_byte_count)
                {
                    src--;
                    dest--;
                    --count;
                    break;
                }
                *dest++ = *src++;
                count.bytes++;
            }
        }
        if (force_null || *src == 0)
        {
            *dest = 0;
            ++count;
        }
        return count;
    }

    const int TitleFPointerOffset = 2;
    const int TitleFPointerChars = 26 + 2;

#define Between(min, i, max) ((min) <= (i) && (i) <= (max))
    int TransformTitleFPointerChar(const char *&c)
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

#warning TODO: Generate programmatically
    int GetSearchPointerIndex(const NonMBChar *searchstring, int pointerdepth)
    {
        int pointerindex = 0;
        for (int d = 0; d < pointerdepth; d++)
        {
            int transformed = TransformTitleFPointerChar(searchstring);
            if (transformed < 0)
            {
                pointerindex += TitleFPointerOffset + transformed;
                break;
            }
            for (int md = d + 1; md < pointerdepth; md++)
                transformed *= TitleFPointerChars;
            pointerindex += TitleFPointerOffset + transformed;
        }
        return pointerindex;
    }
}
