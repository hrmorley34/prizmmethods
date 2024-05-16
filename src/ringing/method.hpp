#include "row.hpp"

#ifndef RINGING_METHOD_HPP
#define RINGING_METHOD_HPP

namespace ringing
{
    const int MAX_METHOD_TITLE_LENGTH = 128;
    const int MAX_PLACE_NOTATION_LENGTH = 256;

    struct Method
    {
        int stage;                                   // number of bells
        char title[MAX_METHOD_TITLE_LENGTH];         // null-terminated name of method
        int leadlength;                              // length of place notation
        PlaceNotation pn[MAX_PLACE_NOTATION_LENGTH]; // bitmasks of non-moving bells
        int leadcount;                               // number of leads in a plain course
        BellBitmask huntbells;                       // bitmask of hunt bells

        inline bool IsHuntBell(ringing::Bell bell) const { return (huntbells & (1 << bell)) != 0; }
        inline int PlainCourseLength() const { return leadlength * leadcount; }
    };
}

#endif
