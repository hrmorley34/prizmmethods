#include <fxcg/display.h>
#include <fxcg/keyboard.h>
#include <fxcg/system.h>
#include <limits.h>
#include "ringing/method.hpp"
#include "ringing/filereader.hpp"
#include "screenstate.hpp"
#include "methodrender.cpp.hpp"

class MethodScreen
{
    ringing::Method method;
    static const int border = 3;
    static const int topborder = 7; // extra space under the title

    int methodXOffset;
    static const int maxXOffset = border; // furthest left allowed to scroll
    int minXOffset;                       // furthest right allowed to scroll

    int methodYOffset;
    static const int initialMaxYOffset = 18 + 18 + topborder; // status bar + title + border
    int maxYOffset;                                           // furthest down allowed to scroll
    int minYOffset;                                           // furthest up allowed to scroll

    methodrender::LineStyle styles[ringing::MAX_BELLS];

    static const color_t TextColour = COLOR_BLACK;
    static const color_t BgColour = COLOR_WHITE;

    static const int scrollXSmall = methodrender::RowWidth;
    static const int scrollXPage = (int)(LCD_WIDTH_PX / scrollXSmall) * scrollXSmall;
    static const int scrollYSmall = methodrender::RowHeight;
    static const int scrollYPage = (int)(LCD_HEIGHT_PX / scrollYSmall) * scrollYSmall;

    void ResetPos()
    {
        minXOffset = LCD_WIDTH_PX - border - methodrender::GetMethodWidth(method);
        minYOffset = LCD_HEIGHT_PX - border - methodrender::RowHeight * method.stage;
        maxYOffset = initialMaxYOffset;
        if (maxYOffset < minYOffset)
            maxYOffset = minYOffset;

        methodXOffset = maxXOffset;
        methodYOffset = maxYOffset;
    }

    void DrawTitle() const
    {
        int x = 0;
        int y = 0;
        PrintMini(&x, &y, method.title, 0x00, LCD_WIDTH_PX, 0, 0, TextColour, BgColour, 1, 0);
    }

    void DrawMethod() const
    {
        int x = methodXOffset + methodrender::RowWidth / 2;

        methodrender::PrintMethod(x, methodYOffset, method, styles);
    }

    void ResetStyles()
    {
        methodrender::CreateStyles(method, styles);
    }
    void ModifyStyles_SetDisplayMode(methodrender::LineDisplayMode display)
    {
        methodrender::ModifyStyles_SetDisplayMode(method, styles, display);
    }
    void ModifyStyles_HideDigits()
    {
        methodrender::ModifyStyles_HideDigits(method, styles);
    }
    void ModifyStyles_SetHiddenDisplayMode(methodrender::LineDisplayMode display)
    {
        methodrender::ModifyStyles_SetHiddenDisplayMode(method, styles, display);
    }

public:
    void Setup()
    {
        Bdisp_EnableColor(1);
        Bkey_SetAllFlags(0x80);    // disable shift+4 for catalog
        EnableDisplayHeader(2, 1); // Let GetKey draw status area

        methodrender::Setup_LineSymbols();

        ResetPos();
        ResetStyles();
    }

    void Draw() const
    {
        Bdisp_AllClr_VRAM();

        DrawMethod();
        DrawTitle();

        EnableDisplayHeader(2, 1);
    }

private:
    bool CycleBell(ringing::Bell bell)
    {
        if (bell < 0 || bell >= method.stage)
            return false;
        styles[bell].CycleDisplayMode();
        return true;
    }

public:
    ScreenState HandleKey(const int key)
    {
        switch (key)
        {
        case KEY_CTRL_EXIT:
            return ScreenState::Search;

        case KEY_CTRL_PAGEUP:
            methodYOffset += scrollYPage - scrollYSmall; // and fall through
        case KEY_CTRL_UP:
            methodYOffset += scrollYSmall;
            if (methodYOffset > maxYOffset)
                methodYOffset = maxYOffset;
            break;
        case KEY_CTRL_PAGEDOWN:
            methodYOffset -= scrollYPage - scrollYSmall; // and fall through
        case KEY_CTRL_DOWN:
            methodYOffset -= scrollYSmall;
            if (methodYOffset < minYOffset)
                methodYOffset = minYOffset;
            break;
        case KEY_SHIFT_LEFT:
            methodXOffset += scrollXPage - scrollXSmall; // and fall through
        case KEY_CTRL_LEFT:
            methodXOffset += scrollXSmall;
            if (methodXOffset > maxXOffset)
                methodXOffset = maxXOffset;
            break;
        case KEY_SHIFT_RIGHT:
            methodXOffset -= scrollXPage - scrollXSmall; // and fall through
        case KEY_CTRL_RIGHT:
            methodXOffset -= scrollXSmall;
            if (methodXOffset < minXOffset)
                methodXOffset = minXOffset;
            break;

        case KEY_CHAR_1:
        case KEY_CHAR_2:
        case KEY_CHAR_3:
        case KEY_CHAR_4:
        case KEY_CHAR_5:
        case KEY_CHAR_6:
        case KEY_CHAR_7:
        case KEY_CHAR_8:
        case KEY_CHAR_9:
            CycleBell(key - KEY_CHAR_1);
            break;
        case KEY_CHAR_0:
            CycleBell(9);
            break;
        case KEY_CHAR_DP: // next to 0
            CycleBell(10);
            break;
        case KEY_CHAR_EXP: // next-but-one to 0
            CycleBell(11);
            break;

        case KEY_CTRL_F1:
        case KEY_CTRL_AC:
            ResetStyles();
            break;

        case KEY_CTRL_F2:
            ModifyStyles_SetDisplayMode(methodrender::LineDisplayMode::ColourDigit);
            break;
        case KEY_CTRL_F3:
            ModifyStyles_SetDisplayMode(methodrender::LineDisplayMode::ColourLine);
            break;
        case KEY_CTRL_F4:
            ModifyStyles_HideDigits();
            break;
        case KEY_CTRL_F5:
            ModifyStyles_SetHiddenDisplayMode(methodrender::LineDisplayMode::ColourDigit);
            break;
        case KEY_CTRL_F6:
            ModifyStyles_SetDisplayMode(methodrender::LineDisplayMode::None);
            break;

        default:
            break;
        }
        return ScreenState::DrawMethod;
    }

    bool ReadMethodFrom(ringing::FileReader &mf)
    {
        return mf.ReadMethod(method);
    }

    void CopyMethodFrom(const ringing::Method &method)
    {
        this->method = method;
    }
};
