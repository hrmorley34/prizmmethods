#include "row.hpp"

namespace ringing
{
    bool ParsePlaceNotation(const int stage, const PlaceNotation pn, ChangeDirection *const directions, ChangeDirection *const backdirections, Bell *const bells)
    {
        if (stage > MAX_BELLS)
            return false;
        bool swap_prev = false, swap_cur;
        for (int i = 0; i < stage; i++)
        {
            swap_cur = (pn & (1 << i)) == 0;
            if (swap_prev)
            {
                if (!swap_cur)
                    return false;
                if (directions != nullptr)
                {
                    directions[i - 1] = ChangeDirection::Up;
                    directions[i] = ChangeDirection::Down;
                }
                if (backdirections != nullptr)
                {
                    backdirections[i] = ChangeDirection::Up;
                    backdirections[i - 1] = ChangeDirection::Down;
                }
                if (bells != nullptr)
                {
                    Bell temp = bells[i - 1];
                    bells[i - 1] = bells[i];
                    bells[i] = temp;
                }
                swap_prev = false;
            }
            else if (swap_cur)
                swap_prev = true;
            else
            {
                if (directions != nullptr)
                    directions[i] = ChangeDirection::Place;
                if (backdirections != nullptr)
                    backdirections[i] = ChangeDirection::Place;
                swap_prev = false;
            }
        }
        if (swap_prev)
            return false;
        return true;
    }

    bool Row::IsRounds() const
    {
        for (int i = 0; i < stage; i++)
            if (row[i] != i)
                return false;
        return true;
    }

    Row Row::Rounds(const int stage)
    {
        if (stage <= 0 || stage > MAX_BELLS)
            return Invalid();
        Row row;
        row.stage = stage;
        for (Bell i = 0; i < stage; i++)
            row.row[i] = i;
        return row;
    }

    void Row::ApplyPn(const PlaceNotation pn, ChangeDirection *const directions, ChangeDirection *const backdirections)
    {
        if (!ParsePlaceNotation(this->stage, pn, directions, backdirections, this->row))
            this->Invalidate();
    }

    Row Row::AddPn(const PlaceNotation pn, ChangeDirection *const directions, ChangeDirection *const backdirections) const
    {
        Row new_row = Row(this);
        new_row.ApplyPn(pn, directions, backdirections);
        return new_row;
    }
}
