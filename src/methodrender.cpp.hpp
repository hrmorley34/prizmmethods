#include <fxcg/display.h>
#include "charset/charset.hpp"
#include "ringing/row.hpp"
#include "ringing/method.hpp"
#include "vram.hpp"

namespace methodrender
{
    const charset::NonMBChar LineChars[ringing::MAX_BELLS] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'E', 'T', 'A', 'B', 'C', 'D'};
    unsigned short LineGlyphWidths[ringing::MAX_BELLS];
    void *LineGlyphs[ringing::MAX_BELLS];
    const color_t LineColours[ringing::MAX_BELLS] = {
        COLOR_RED, COLOR_BLUE, COLOR_LIME, COLOR_MAGENTA,
        COLOR_YELLOW, COLOR_CYAN, COLOR_GREEN, COLOR_DARKORANGE,
        COLOR_PINK, COLOR_LIGHTBLUE, COLOR_PURPLE, COLOR_BROWN,
        COLOR_DARKTURQUOISE, COLOR_GRAY, COLOR_MAROON, COLOR_DARKGRAY};
    const color_t DefaultTextColour = COLOR_BLACK;

    const int HuntThickness = 2;
    const int WorkingThickness = 4;

    enum LineDisplayMode
    {
        None = 0,
        PlainDigit = 0b1,
        ColourDigit = 0b10,
        AnyDigit = PlainDigit | ColourDigit,
        ColourLine = 0b100,
        ColourLineDigit = ColourLine | ColourDigit,
        AnyLine = ColourLine,
    };

    struct LineStyle
    {
        LineDisplayMode display;
        color_t colour;
        int thickness;

        inline bool GetTextDisplay() const
        {
            return (display & LineDisplayMode::AnyDigit) != 0;
        }
        inline color_t GetTextColour() const
        {
            return (display & LineDisplayMode::ColourDigit) != 0 ? colour : DefaultTextColour;
        }
        inline int GetLineThickness() const
        {
            return (display & LineDisplayMode::AnyLine) != 0 ? thickness : 0;
        }
        inline color_t GetLineColour() const
        {
            return colour;
        }

        void CycleDisplayMode()
        {
            switch (display)
            {
            case LineDisplayMode::None:
                display = LineDisplayMode::PlainDigit;
                break;
            case LineDisplayMode::PlainDigit:
                display = LineDisplayMode::ColourDigit;
                break;
            case LineDisplayMode::ColourDigit:
                display = LineDisplayMode::ColourLine;
                break;
            case LineDisplayMode::ColourLine:
                display = LineDisplayMode::ColourLineDigit;
                break;
            default:
                display = LineDisplayMode::None;
            }
        }
    };

    void Setup_LineSymbols()
    {
        for (int index = 0; index < ringing::MAX_BELLS; index++)
        {
            LineGlyphs[index] = GetMiniGlyphPtr(LineChars[index], &LineGlyphWidths[index]);
        }
    }

    const int RowHeight = 18;
    const int RowWidth = RowHeight;
    const int RowVCentre = RowHeight / 2;
    const int LineVAdj = -2;
    // Babylonian constexpr approximation for 100 * sqrt(RowHeight^2 + RowWidth^2) / RowHeight
    const int ThicknessModifier100 = (int)(100. * ((float)RowHeight + 0.5 * (float)RowWidth * (float)RowWidth / (float)RowHeight) / (float)RowHeight);

    void DrawBackLine(const int ex, const int ey, const int base_thickness, const ringing::ChangeDirection direction, const color_t colour)
    {
        const int startdx = -RowWidth;
        int mindx = startdx - 2;
        if (ex + mindx < 0)
            mindx = -ex;
        const int enddx = 0;
        int maxdx = enddx + 2;
        if (ex + maxdx > LCD_WIDTH_PX)
            maxdx = LCD_WIDTH_PX - ex;

        int fullminy = ey + RowVCentre + LineVAdj - base_thickness / 2;
        int fullmaxy = fullminy + base_thickness;
        int thickness100;
        switch (direction)
        {
        case ringing::ChangeDirection::Up:
            fullmaxy += RowHeight;
            thickness100 = ThicknessModifier100 * base_thickness;
            break;
        case ringing::ChangeDirection::Down:
            fullminy -= RowHeight;
            thickness100 = ThicknessModifier100 * base_thickness;
            break;
        default:
            thickness100 = 100 * base_thickness;
            break;
        }
        if (fullminy < 0)
            fullminy = 0;
        if (fullmaxy > LCD_HEIGHT_PX)
            fullmaxy = LCD_HEIGHT_PX;

        for (int dx = mindx; dx < maxdx; dx++)
        {
            int draw_thickness100 = thickness100;
            int basedy = -dx * (int)direction * RowHeight / RowWidth;
            if (dx < startdx)
            {
                draw_thickness100 -= 200 * RowHeight * (startdx - dx) / RowWidth;
                basedy = (int)direction * RowHeight;
            }
            if (dx > enddx)
            {
                draw_thickness100 -= 200 * RowHeight * (dx - enddx) / RowWidth;
                basedy = 0;
            }

            int basey = ey + RowVCentre + LineVAdj + basedy - draw_thickness100 / 200;

            int miny = basey;
            if (miny < fullminy)
                miny = fullminy;
            int maxy = basey + draw_thickness100 / 100;
            if (maxy > fullmaxy)
                maxy = fullmaxy;

            for (int y = miny; y < maxy; y++)
                VRAMpos(ex + dx, y) = colour;
        }
    }

    void PrintBell(const int cx, const int sy, const ringing::Bell bell, const color_t colour)
    {
        void *const glyph = LineGlyphs[bell];
        const unsigned short width = LineGlyphWidths[bell];
        const int x = cx - width / 2;

        PrintMiniGlyph(x, sy, glyph, 0x42, width, 0, 0, 0, 0, colour, 0, 0);
    }

    void PrintRow(const int cx, int y, const int stage, const ringing::Bell row[], const LineStyle styles[])
    {
        for (int i = stage - 1; i >= 0; i--)
        {
            auto bell = row[i];
            LineStyle style = styles[bell];

            if (style.GetTextDisplay())
                PrintBell(cx, y, bell, style.GetTextColour());

            y += RowHeight;
        }
    }

    void PrintBackLines(const int ex, int y, const int stage, const ringing::Bell row[], ringing::ChangeDirection backdirections[], const LineStyle styles[])
    {
        for (int i = stage - 1; i >= 0; i--)
        {
            auto bell = row[i];
            auto dir = backdirections[i];
            LineStyle style = styles[bell];
            auto lineThickness = style.GetLineThickness();

            if (lineThickness > 0)
                DrawBackLine(ex, y, lineThickness, dir, style.GetLineColour());

            y += RowHeight;
        }
    }

    void PrintFirstRow(const int &cx, const int sy, const ringing::Row &row, const LineStyle styles[])
    {
        PrintRow(cx, sy, row.stage, row.row, styles);
    }

    void UpdateAndPrintPn(int &cx, const int sy, ringing::Row &row, const ringing::PlaceNotation pn, const LineStyle styles[])
    {
        ringing::ChangeDirection backdirections[row.stage];
        row.ApplyPn(pn, nullptr, backdirections);
        cx += RowWidth;
        PrintRow(cx, sy, row.stage, row.row, styles);
        PrintBackLines(cx, sy, row.stage, row.row, backdirections, styles);
    }

    void CreateStyles(const ringing::Method &method, LineStyle *styles)
    {
        const color_t *colourptr = &LineColours[0];
        bool firstwb = true;
        for (ringing::Bell bell = 0; bell < method.stage; bell++)
        {
            styles->colour = *colourptr++;
            if (method.IsHuntBell(bell))
            {
                styles->thickness = HuntThickness;
                styles->display = LineDisplayMode::ColourLine;
            }
            else
            {
                styles->thickness = WorkingThickness;
                if (firstwb)
                {
                    firstwb = false;
                    styles->display = LineDisplayMode::ColourLine;
                }
                else
                    styles->display = LineDisplayMode::ColourDigit;
            }
            styles++;
        }
    }

    void ModifyStyles_SetDisplayMode(const ringing::Method &method, LineStyle *styles, LineDisplayMode display)
    {
        for (ringing::Bell bell = 0; bell < method.stage; bell++)
            (styles++)->display = display;
    }
    void ModifyStyles_HideDigits(const ringing::Method &method, LineStyle *styles)
    {
        for (ringing::Bell bell = 0; bell < method.stage; bell++)
        {
            styles->display = (methodrender::LineDisplayMode)(styles->display & ~LineDisplayMode::AnyDigit);
            styles++;
        }
    }
    void ModifyStyles_SetHiddenDisplayMode(const ringing::Method &method, LineStyle *styles, LineDisplayMode display)
    {
        for (ringing::Bell bell = 0; bell < method.stage; bell++)
        {
            if (styles->display == LineDisplayMode::None)
                styles->display = display;
            styles++;
        }
    }

    int GetMethodWidth(const ringing::Method &method)
    {
        // +1 for the rounds
        return RowWidth * (method.PlainCourseLength() + 1);
    }

    // Render the method. Returns true if the end of the method was reached.
    bool PrintMethod(int &cx, const int sy, const ringing::Method &method, const LineStyle *const styles)
    {
        // if (cx - RowWidth / 2 >= LCD_WIDTH_PX ||
        //     sy >= LCD_HEIGHT_PX || sy + RowHeight * method.stage < 0)
        //     return false;

        // TODO: Speed up by starting with named leadheads?

        ringing::Row row = ringing::Row::Rounds(method.stage);
        // Row is visible if the right edge is past the left of the screen
        if (cx + RowWidth / 2 >= 0)
            PrintFirstRow(cx, sy, row, styles);
        int pn_i = 0;
        // Next row is invisible if its right edge is not past the left of the screen
        // Need to consider next one due to line drawing.
        while (cx + RowWidth + RowWidth / 2 < 0)
        {
            row.ApplyPn(method.pn[pn_i++]);
            cx += RowWidth;
            pn_i %= method.leadlength;

            if (pn_i == 0 && row.IsRounds())
                return true;
        }

        // Row is visible if left edge isn't past the right of the screen
        while (cx - RowWidth / 2 < LCD_WIDTH_PX)
        {
            UpdateAndPrintPn(cx, sy, row, method.pn[pn_i++], styles);
            pn_i %= method.leadlength;

            if (pn_i == 0 && row.IsRounds())
                return true;
        }

        return false;
    }
}
