#include <fxcg/file.h>
#include "ringing/row.hpp"
#include "ringing/filereader.hpp"

char METHOD_FILE[] = "\\\\fls0\\methods\\methods-X.ccml";
char *const METHOD_FILE_STAGE_CHAR = METHOD_FILE + 23;
unsigned short METHOD_FILE_W[sizeof(METHOD_FILE)];

const char METHOD_FILE_STAGE_CHARS[ringing::MAX_BELLS + 1] = {
    0, '1', '2', '3', '4', '5', '6', '7', '8',
    '9', '0', 'E', 'T', 'A', 'B', 'C', 'D'};

bool PrepareLoadMethodFile(int stage)
{
    if (stage <= 0 || stage > ringing::MAX_BELLS)
        return false;
    *METHOD_FILE_STAGE_CHAR = METHOD_FILE_STAGE_CHARS[stage];

    Bfile_StrToName_ncpy(METHOD_FILE_W, METHOD_FILE, sizeof(METHOD_FILE));
    return true;
}

bool LoadMethodFile(ringing::FileReader &mf, int stage)
{
    if (!PrepareLoadMethodFile(stage))
        return false;
    mf.Close();
    if (!mf.TryOpen(METHOD_FILE_W))
        return false;
    return true;
}

// The stages for which we expect there to be files.
const int STAGES_COUNT = 15;
const int STAGES[STAGES_COUNT] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
const char *STAGE_HEADERS[STAGES_COUNT] = {
    "Two", "Singles", "Minimus", "Doubles", "Minor", "Triples",
    "Major", "Caters", "Royal", "Cinques", "Maximus", "Sextuples",
    "Fourteen", "Septuples", "Sixteen"};
const int INITIAL_STAGE_INDEX = 4; // Minor
