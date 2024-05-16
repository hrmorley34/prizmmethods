#include <fxcg/display.h>
#include <fxcg/keyboard.h>
#include <fxcg/system.h>
#include <string.h>
#include "ringing/method.hpp"
#include "ringing/search.hpp"
#include "ringing/filereader.hpp"
#include "keyboardmode.hpp"
#include "screenstate.hpp"

#include "methodfiles.cpp.hpp"

struct SearchResult
{
    static const int MAX_DISPLAY_METHOD_TITLE_LENGTH = 22; // Display fits 21 characters, plus one overflow, excluding null

    // position in the method file, or -1 for empty result
    int file_pos;
    // (cropped) title of the method
    char title[MAX_DISPLAY_METHOD_TITLE_LENGTH + 1];

    bool Exists() const { return file_pos >= 0; }
};

class SearchScreen
{
    static const int MAX_SEARCH_LENGTH = 16;
    char search_text[MAX_SEARCH_LENGTH + 1]; // +1 for null
    int search_cursor = 0;
    static const int MAX_SEARCH_RESULTS_PER_PAGE = 7;
    SearchResult results[MAX_SEARCH_RESULTS_PER_PAGE];
    int selected_result = 0;
    int selected_page = 0;

    int cur_stage_index;
    int new_stage_index = INITIAL_STAGE_INDEX;
    bool good_read;

    ringing::FileReader *mf;
    static const int MAX_SEARCH_PAGES = 20;
    // position in the file of the start of each page, or 0 for unset, or -1 for end of results
    int file_page_positions[MAX_SEARCH_PAGES + 1] = {0};
    int search_pages;

    static const int font_width = 18;
    static const int font_height = 24;
    static const int left = 0;
    static const int top = 0; // PrintCXY automatically includes status bar

    static const color_t bg_default = COLOR_WHITE;
    static const color_t bg_selected = COLOR_LIGHTGRAY;
    static const color_t fg_default = COLOR_BLACK;
    static const color_t fg_search = COLOR_BLUE;
    static const color_t fg_space = COLOR_DARKGRAY;
    static const color_t fg_missing = COLOR_RED;
    static const color_t fg_missingspace = COLOR_PINK;

    static const color_t bg_header = COLOR_LIGHTBLUE;
    static const color_t fg_headerarrow = COLOR_BLUE;
    static const color_t fg_paleheaderarrow = COLOR_SKYBLUE;
    static const color_t fg_header = COLOR_BLACK;

    void DrawResults() const
    {
        int hx = left;
        PrintCXY(hx, top, "< ", TEXT_MODE_NORMAL, -1,
                 cur_stage_index == 0 ? fg_paleheaderarrow : fg_headerarrow, bg_header, 1, 0);
        hx += 2 * font_width;
        PrintCXY(hx, top, STAGE_HEADERS[cur_stage_index], TEXT_MODE_NORMAL, -1,
                 fg_header, bg_header, 1, 0);
        hx += strlen(STAGE_HEADERS[cur_stage_index]) * font_width;
        PrintCXY(hx, top, " >", TEXT_MODE_NORMAL, -1,
                 cur_stage_index == STAGES_COUNT - 1 ? fg_paleheaderarrow : fg_headerarrow, bg_header, 1, 0);

        if (!good_read)
        {
            PrintCXY(left, top + 1 * font_height, "Failed to read", TEXT_MODE_NORMAL, -1, fg_missing, bg_default, 1, 0);
            PrintCXY(left, top + 2 * font_height, "file!", TEXT_MODE_NORMAL, -1, fg_missing, bg_default, 1, 0);
            return;
        }

        const int prefixlength = strlen(search_text);
        char buf[SearchResult::MAX_DISPLAY_METHOD_TITLE_LENGTH] = {0};
        const char *titleptr;
        int i, x, y, trueprefixlength;
        y = top + font_height; // below header
        for (int result_index = 0; result_index < MAX_SEARCH_RESULTS_PER_PAGE; result_index++)
        {
            const SearchResult &result = results[result_index];
            if (!result.Exists())
            {
                if (result_index == 0) // no results
                {
                    PrintCXY(left, y, search_text, TEXT_MODE_NORMAL, -1, fg_missing, bg_default, 1, 0);
                    for (int i = 0; i < prefixlength; i++)
                        if (search_text[i] == ' ') // make spaces in the search text more obvious
                            PrintCXY(left + i * font_width, y, "_", TEXT_MODE_NORMAL, -1, fg_missingspace, bg_default, 1, 0);
                }
                break; // no more results
            }
            titleptr = result.title;

            x = left;
            color_t bg = result_index == selected_result ? bg_selected : bg_default;

            trueprefixlength = 0;
            if (prefixlength > 0)
            {
                i = 0;
                while (i < prefixlength && *titleptr != 0)
                {
                    char c = *(titleptr++);
                    buf[trueprefixlength++] = c;
                    if (!ringing::SkipTitleChar(c))
                        i++;
                }
                buf[trueprefixlength] = 0;

                PrintCXY(x, y, buf, TEXT_MODE_NORMAL, -1, fg_search, bg, 1, 0);
                for (int i = 0; i < trueprefixlength; i++)
                    if (buf[i] == ' ') // make spaces in the search text more obvious
                        PrintCXY(x + i * font_width, y, "_", TEXT_MODE_NORMAL, -1, fg_space, bg, 1, 0);
                x += trueprefixlength * font_width;
            }

            for (i = 0; i < SearchResult::MAX_DISPLAY_METHOD_TITLE_LENGTH - trueprefixlength && *titleptr != 0; i++)
                buf[i] = *(titleptr++);
            buf[i] = 0; // null required here - skipped by the loop

            PrintCXY(x, y, buf, TEXT_MODE_NORMAL, -1, fg_default, bg, 1, 0);

            y += font_height;
        }
    }

    void Prepare()
    {
        if (!good_read || new_stage_index != cur_stage_index)
        {
            good_read = mf != nullptr;
            if (good_read)
                LoadMethodFile(*mf, STAGES[new_stage_index]);
            cur_stage_index = new_stage_index;
        }

        search_text[0] = 0;
        search_cursor = 0;
        selected_result = 0;
        if (good_read)
            good_read = Search();

        SetInputMode(KEYBOARD_MODE_ALPHA_LOCK);
    }

    bool ReadPageEntries()
    {
        mf->Seek(file_page_positions[selected_page]);
        bool end_of_results = false;
        char title[ringing::MAX_METHOD_TITLE_LENGTH];
        for (int i = 0; i < MAX_SEARCH_RESULTS_PER_PAGE; i++)
        {
            if (end_of_results)
            {
                results[i].file_pos = -1;
                continue;
            }

            int pos;
            bool goodread = mf->ReadMethodSummary(&pos, nullptr, title);
            if (!goodread || !ringing::CompareTitles(search_text, title))
            {
                end_of_results = true;
                results[i].file_pos = -1;
                continue;
            }
            results[i].file_pos = pos;
            // No need to check for null/end - length of method.title is longer than results.title
            for (int j = 0; j < SearchResult::MAX_DISPLAY_METHOD_TITLE_LENGTH; j++)
                results[i].title[j] = title[j];
            // guarantee ending null, in case the string needs chopping
            results[i].title[SearchResult::MAX_DISPLAY_METHOD_TITLE_LENGTH] = 0;

            if (mf->EndOfFile())
                end_of_results = true;
        }
        file_page_positions[selected_page + 1] = end_of_results ? -1 : mf->Tell();
        return end_of_results;
    }

    bool Search()
    {
        // int key;
        search_pages = MAX_SEARCH_PAGES; // (make a guess - can be refined)
        selected_page = 0;
        selected_result = 0;
        for (int page = 0; page < MAX_SEARCH_PAGES + 1; page++)
            file_page_positions[page] = 0;
        // PrintXY(1, 1, "  Ready to search", TEXT_MODE_NORMAL, TEXT_COLOR_GREEN);
        // GetKey(&key);
        if (!mf->Search(search_text, &file_page_positions[0]))
        {
            // PrintXY(1, 2, "  Error in search", TEXT_MODE_NORMAL, TEXT_COLOR_RED);
            // do
            // {
            //     GetKey(&key);
            // } while (key != KEY_CTRL_EXIT);
            return false;
        }
        // PrintXY(1, 2, "  Reading from search", TEXT_MODE_NORMAL, TEXT_COLOR_GREEN);
        // GetKey(&key);
        if (ReadPageEntries())
            search_pages = 1;
        return true;
    }

    void GoToPage(int index)
    {
        if (index < 0)
        {
            index = 0;
            selected_result = 0;
        }
        if (index >= search_pages)
        {
            index = search_pages - 1;
            selected_result = MAX_SEARCH_RESULTS_PER_PAGE - 1;
        }
        for (selected_page = 0; selected_page < index; selected_page++)
        {
            if (file_page_positions[selected_page + 1] == 0)
            {
                if (ReadPageEntries()) // we've reached the last page
                {
                    selected_result = MAX_SEARCH_RESULTS_PER_PAGE - 1;

                    goto last_page;
                }
            }
            else if (file_page_positions[selected_page + 1] < 0) // this is the last page
                break;
        }
        if (ReadPageEntries())
        {
        last_page:
            if (!results[0].Exists()) // this page is empty
            {
                selected_page--;
                selected_result = MAX_SEARCH_RESULTS_PER_PAGE - 1;
                ReadPageEntries();
            }
            search_pages = selected_page + 1;
        }
    }

public:
    SearchScreen() {}

    void Setup()
    {
        Bdisp_EnableColor(1);
        Bkey_SetAllFlags(0x80);    // disable shift+4 for catalog
        EnableDisplayHeader(2, 1); // Let GetKey draw status area

        Prepare();
    }

    void Draw() const
    {
        Bdisp_AllClr_VRAM();
        // DisplayStatusArea(); // handled by GetKey
        DrawResults();

        EnableDisplayHeader(2, 1); // Let GetKey draw status area
    }

    int GetSelectedFilePos()
    {
        if (!good_read)
            return -1;
        return results[selected_result].file_pos;
    }

    void SetFileReader(ringing::FileReader *mf)
    {
        this->mf = mf;
    }

private:
    static bool KeyToChar(const int key, char *const c)
    {
        if ((key == ' ') || ('0' <= key && key <= '9') || ('A' <= key && key <= 'Z') || ('a' <= key && key <= 'z'))
        {
            *c = key;
            return true;
        }
        return false;
    }

public:
    ScreenState HandleKey(const int key)
    {
        switch (key)
        {
        case KEY_SHIFT_LEFT:
            if (cur_stage_index == 0) // no change
                break;
            new_stage_index = cur_stage_index - 1;
            if (new_stage_index < 0)
                new_stage_index = 0;
            return ScreenState::ReloadSearch;
        case KEY_SHIFT_RIGHT:
            if (cur_stage_index == STAGES_COUNT - 1) // no change
                break;
            new_stage_index = cur_stage_index + 1;
            if (new_stage_index >= STAGES_COUNT)
                new_stage_index = STAGES_COUNT - 1;
            return ScreenState::ReloadSearch;

        case KEY_CTRL_DEL:
            if (search_cursor > 0)
                search_text[--search_cursor] = 0;
            Search();
            break;
        case KEY_CTRL_AC:
            search_text[search_cursor = 0] = 0;
            Search();
            break;

        case KEY_CTRL_UP:
            selected_result--;
            goto check_page_wrap;
        case KEY_CTRL_DOWN:
            selected_result++;
            goto check_page_wrap;
        check_page_wrap:
        {
            int pagechange = selected_result / MAX_SEARCH_RESULTS_PER_PAGE;
            selected_result %= MAX_SEARCH_RESULTS_PER_PAGE;
            if (selected_result < 0) // fix negative remainders
            {
                pagechange--;
                selected_result += MAX_SEARCH_RESULTS_PER_PAGE;
            }
            if (pagechange != 0)
            {
                selected_page += pagechange;
                goto page_wrap;
            }
            goto check_result_exists;
        }
        case KEY_CTRL_PAGEUP:
            selected_page--;
            goto page_wrap;
        case KEY_CTRL_PAGEDOWN:
            selected_page++;
            goto page_wrap;
        page_wrap:
            GoToPage(selected_page);
        check_result_exists:
            // Don't allow selecting non-existant results
            while (selected_result > 0 && !results[selected_result].Exists())
                selected_result--;
            break;

        case KEY_CTRL_EXE:
            return ScreenState::LoadMethod;

        default:
        {
            char c;
            if (KeyToChar(key, &c))
            {
                if (search_cursor < MAX_SEARCH_LENGTH)
                {
                    search_text[search_cursor++] = c;
                    search_text[search_cursor] = 0;
                }
                Search();
            }
            break;
        }
        }

        // // Make typing letters easier
        // if (GetInputMode() == 0)
        //     SetInputMode(KEYBOARD_MODE_ALPHA_LOCK);

        return ScreenState::Search;
    }
};
