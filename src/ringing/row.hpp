#include "../stdint.h"

#ifndef RINGING_ROW_HPP
#define RINGING_ROW_HPP

namespace ringing
{
    const int MAX_BELLS = 16;

    typedef uint16_t BellBitmask;
    static_assert(MAX_BELLS <= sizeof(BellBitmask) * 8, "MAX_BELLS must fit in BellBitmask");
    typedef BellBitmask PlaceNotation;
    typedef uint8_t Bell;

    enum ChangeDirection : int
    {
        Down = -1,
        Place = 0,
        Up = 1,
    };

    bool ParsePlaceNotation(int stage, PlaceNotation pn, ChangeDirection *directions = nullptr, ChangeDirection *backdirections = nullptr, Bell *bells = nullptr);

    struct Row
    {
        int stage;
        Bell row[MAX_BELLS];

        Row() { this->stage = 0; }

        Row(int stage, const Bell row[])
        {
            if (stage > MAX_BELLS || stage < 0)
                stage = 0;
            this->stage = stage;
            for (int i = 0; i < stage; i++)
                this->row[i] = row[i];
        }

        Row(const Row *const row) : Row(row->stage, row->row) {}

        static inline Row Invalid() { return Row(); }
        static Row Rounds(const int stage);

        inline void Invalidate() { stage = 0; }
        inline bool IsValid() const { return stage > 0; }
        bool IsRounds() const;

        void ApplyPn(PlaceNotation pn, ChangeDirection *directions = nullptr, ChangeDirection *backdirections = nullptr);
        Row AddPn(PlaceNotation pn, ChangeDirection *directions = nullptr, ChangeDirection *backdirections = nullptr) const;
    };
}

#endif
