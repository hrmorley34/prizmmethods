#include <fxcg/display.h>
#include <fxcg/keyboard.h>
#include <fxcg/system.h>
#include "screen_method.cpp.hpp"
#include "screen_search.cpp.hpp"

#include "ringing/method.cpp"
#include "ringing/row.cpp"
#include "ringing/search.cpp"
#include "ringing/filereader.cpp"
#include "vram.cpp.hpp"
#include "test_methods.hpp"

ringing::FileReader mf;
SearchScreen ss;
MethodScreen ms;

void QuitHandler()
{
    mf.Close();
}

int main(void)
{
    int key;

    Bdisp_AllClr_VRAM();
    Setup_VRAM();

    SetQuitHandler(QuitHandler);

    ss.SetFileReader(&mf);

    // ms.CopyMethodFrom(PlainBob6);
    // ScreenState state = ScreenState::DrawMethod;

    ScreenState state = ScreenState::Search;

    while (true)
    {
        switch (state)
        {
        case ScreenState::ReloadSearch:
        case ScreenState::Search:
            ss.Setup();
            do
            {
                ss.Draw();
                GetKey(&key);
                state = ss.HandleKey(key);
            } while (state == ScreenState::Search);
            break;
        case ScreenState::LoadMethod:
        {
            int pos = ss.GetSelectedFilePos();
            if (pos < 0)
            {
                state = ScreenState::MethodReadError;
                break;
            }
            mf.Seek(pos);
        }
            if (!ms.ReadMethodFrom(mf))
            {
                state = ScreenState::MethodReadError;
                break;
            }
            state = ScreenState::DrawMethod; // Fall-through
        case ScreenState::DrawMethod:
            ms.Setup();
            do
            {
                ms.Draw();
                GetKey(&key);
                state = ms.HandleKey(key);
            } while (state == ScreenState::DrawMethod);
            // state = ScreenState::DrawMethod;
            break;
        case ScreenState::MethodReadError:
            PrintXY(1, 1, "  Failed to read method.", TEXT_MODE_NORMAL, TEXT_COLOR_RED);
            PrintXY(1, 2, "  Press      to return.", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
            PrintXY(7, 2, "  EXIT", TEXT_MODE_NORMAL, TEXT_COLOR_BLUE);
            do
            {
                GetKey(&key);
            } while (key != KEY_CTRL_EXIT);
            state = ScreenState::Search;
            break;
        default:
            PrintXY(1, 1, "  An error occured.", TEXT_MODE_NORMAL, TEXT_COLOR_RED);
            GetKey(&key);
            state = ScreenState::Search;
        }
    }
}
