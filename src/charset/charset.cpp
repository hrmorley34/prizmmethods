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

    int GetSearchPointerIndex(const NonMBChar *searchstring, int pointerdepth)
    {
        int pindex = 0;
        for (int kindex = 0; kindex < pointerdepth; kindex++)
        {
            const SearchIndex k = ReadSearchCharPtr(searchstring);
            if (k < 0) // end of string
                return pindex;
            const int depth = pointerdepth - kindex - 1;
            const int layersize = GetJumpDepth(depth);
            for (int j = 0; j < jumpCharCount; j++)
            {
                if (k == j)
                {
                    if (IsSearchStop(k))
                        return pindex;
                    break;
                }
                pindex += IsSearchStop(j) ? 1 : layersize;
            }
        }
        return pindex;
    }
}
