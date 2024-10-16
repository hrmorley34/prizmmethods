NonMBChar ReadSearchChar(const MBChar *&c)
{
    if (*c == '\0')
        return '\0';
    switch (*c++)
    {
        case '0':
            return '0';
        case '1':
            return '1';
        case '2':
            return '2';
        case '3':
            return '3';
        case '4':
            return '4';
        case '5':
            return '5';
        case '6':
            return '6';
        case '7':
            return '7';
        case '8':
            return '8';
        case '9':
            return '9';
        case '\x1a':
        case 'A':
        case 'a':
            return 'A';
        case '\x1b':
        case 'B':
        case 'b':
            return 'B';
        case '\x1c':
        case 'C':
        case 'c':
            return 'C';
        case '\x1d':
        case 'D':
        case 'd':
            return 'D';
        case '\x0b':
        case '\x1e':
        case 'E':
        case 'e':
            return 'E';
        case '\x01':
        case '\x1f':
        case 'F':
        case 'f':
            return 'F';
        case '\x08':
        case 'G':
        case 'g':
            return 'G';
        case 'H':
        case 'h':
            return 'H';
        case 'I':
        case 'i':
            return 'I';
        case 'J':
        case 'j':
            return 'J';
        case '\x06':
        case 'K':
        case 'k':
            return 'K';
        case 'L':
        case 'l':
            return 'L';
        case '\x05':
        case '\x07':
        case 'M':
        case 'm':
            return 'M';
        case '\x03':
        case 'N':
        case 'n':
            return 'N';
        case 'O':
        case 'o':
            return 'O';
        case '\x02':
        case '\x0a':
        case 'P':
        case 'p':
            return 'P';
        case 'Q':
        case 'q':
            return 'Q';
        case 'R':
        case 'r':
        case '\xcd':
            return 'R';
        case 'S':
        case 's':
            return 'S';
        case '\x09':
        case 'T':
        case 't':
            return 'T';
        case 'U':
        case 'u':
            return 'U';
        case 'V':
        case 'v':
            return 'V';
        case 'W':
        case 'w':
            return 'W';
        case 'X':
        case 'x':
        case '\x90':
        case '\xc2':
        case '\xcb':
            return 'X';
        case 'Y':
        case 'y':
        case '\xc3':
        case '\xcc':
            return 'Y';
        case 'Z':
        case 'z':
            return 'Z';
        case '\x7f':
            switch (*c++)
            {
                case '\x50':
                    return 'I';
                case '\xc7':
                    return 'P';
                default:
                    return ' ';
            }
        case '\xe5':
            switch (*c++)
            {
                case '\xc0':
                case '\xcd':
                case '\xd0':
                    return '0';
                case '\xc1':
                case '\xce':
                case '\xd1':
                case '\xf0':
                    return '1';
                case '\xc2':
                case '\xcf':
                case '\xd2':
                case '\xf1':
                    return '2';
                case '\xc3':
                case '\xd3':
                case '\xdf':
                case '\xf2':
                    return '3';
                case '\xc4':
                case '\xd4':
                case '\xf3':
                    return '4';
                case '\xc5':
                case '\xd5':
                case '\xf4':
                    return '5';
                case '\xc6':
                case '\xd6':
                case '\xf5':
                    return '6';
                case '\xc7':
                case '\xd7':
                case '\xfb':
                    return '7';
                case '\xc8':
                case '\xd8':
                case '\xfc':
                    return '8';
                case '\xc9':
                case '\xd9':
                case '\xfd':
                    return '9';
                case '\x01':
                case '\x02':
                case '\x03':
                case '\x04':
                case '\x05':
                case '\x06':
                case '\x21':
                case '\x22':
                case '\x9f':
                    return 'A';
                case '\x08':
                case '\x23':
                case '\x24':
                    return 'C';
                case '\x26':
                    return 'D';
                case '\x09':
                case '\x0a':
                case '\x0b':
                case '\x0c':
                case '\x27':
                case '\x28':
                case '\xb0':
                    return 'E';
                case '\x0d':
                case '\x0e':
                case '\x0f':
                case '\x10':
                    return 'I';
                case '\x12':
                case '\x2a':
                case '\x2b':
                case '\xde':
                    return 'N';
                case '\x13':
                case '\x14':
                case '\x15':
                case '\x16':
                case '\x17':
                case '\x18':
                case '\x2c':
                case '\xa2':
                    return 'O';
                case '\xb1':
                    return 'P';
                case '\x2d':
                case '\xb2':
                    return 'R';
                case '\x2e':
                case '\x2f':
                    return 'S';
                case '\x30':
                    return 'T';
                case '\x19':
                case '\x1a':
                case '\x1b':
                case '\x1c':
                case '\x31':
                case '\x32':
                    return 'U';
                case '\xb3':
                    return 'X';
                case '\x1d':
                case '\x20':
                case '\xb4':
                    return 'Y';
                case '\x33':
                case '\x34':
                case '\x35':
                    return 'Z';
                default:
                    return ' ';
            }
        case '\xe6':
            switch (*c++)
            {
                case '\x01':
                case '\x02':
                case '\x03':
                case '\x04':
                case '\x05':
                case '\x06':
                case '\x21':
                case '\x22':
                    return 'A';
                case '\x08':
                case '\x23':
                case '\x24':
                    return 'C';
                case '\x26':
                    return 'D';
                case '\x09':
                case '\x0a':
                case '\x0b':
                case '\x0c':
                case '\x27':
                case '\x28':
                    return 'E';
                case '\x0d':
                case '\x0e':
                case '\x0f':
                case '\x10':
                    return 'I';
                case '\x12':
                case '\x2a':
                case '\x2b':
                    return 'N';
                case '\x13':
                case '\x14':
                case '\x15':
                case '\x16':
                case '\x17':
                case '\x18':
                case '\x2c':
                    return 'O';
                case '\x2d':
                    return 'R';
                case '\x1f':
                case '\x2e':
                case '\x2f':
                    return 'S';
                case '\x30':
                    return 'T';
                case '\x19':
                case '\x1a':
                case '\x1b':
                case '\x1c':
                case '\x31':
                case '\x32':
                    return 'U';
                case '\x1d':
                case '\x20':
                    return 'Y';
                case '\x33':
                case '\x34':
                case '\x35':
                    return 'Z';
                default:
                    return ' ';
            }
        case '\xe7':
            switch (*c++)
            {
                case '\x30':
                    return '0';
                case '\x31':
                    return '1';
                case '\x32':
                    return '2';
                case '\x33':
                    return '3';
                case '\x34':
                    return '4';
                case '\x35':
                    return '5';
                case '\x36':
                    return '6';
                case '\x37':
                    return '7';
                case '\x38':
                    return '8';
                case '\x39':
                    return '9';
                case '\x61':
                case '\x89':
                case '\xae':
                    return 'A';
                case '\xaf':
                    return 'B';
                case '\x65':
                case '\x96':
                    return 'E';
                case '\x95':
                    return 'F';
                case '\xa2':
                case '\xa6':
                    return 'G';
                case '\x68':
                case '\x85':
                    return 'H';
                case '\x6b':
                case '\x98':
                    return 'K';
                case '\x6c':
                    return 'L';
                case '\x6d':
                    return 'M';
                case '\x6e':
                case '\xad':
                    return 'N';
                case '\x6f':
                    return 'O';
                case '\x70':
                case '\xab':
                    return 'P';
                case '\x9a':
                    return 'R';
                case '\x73':
                    return 'S';
                case '\x74':
                case '\xa5':
                    return 'T';
                case '\x90':
                    return 'U';
                case '\x78':
                    return 'X';
                default:
                    return ' ';
            }
        case '\xf7':
            switch (*c++)
            {
                default:
                    return ' ';
            }
        case '\xf9':
            switch (*c++)
            {
                default:
                    return ' ';
            }
        default:
            return ' ';
    }
}

SearchIndex ReadSearchCharPtr(const MBChar *&c)
{
    if (*c == '\0')
        return -1;
    switch (*c++)
    {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return 1;
        case 'A':
        case 'a':
            return 2;
        case 'B':
        case 'b':
            return 3;
        case 'C':
        case 'c':
            return 4;
        case 'D':
        case 'd':
            return 5;
        case 'E':
        case 'e':
            return 6;
        case 'F':
        case 'f':
            return 7;
        case 'G':
        case 'g':
            return 8;
        case 'H':
        case 'h':
            return 9;
        case 'I':
        case 'i':
            return 10;
        case 'J':
        case 'j':
            return 11;
        case 'K':
        case 'k':
            return 12;
        case 'L':
        case 'l':
            return 13;
        case 'M':
        case 'm':
            return 14;
        case 'N':
        case 'n':
            return 15;
        case 'O':
        case 'o':
            return 16;
        case 'P':
        case 'p':
            return 17;
        case 'Q':
        case 'q':
            return 18;
        case 'R':
        case 'r':
            return 19;
        case 'S':
        case 's':
            return 20;
        case 'T':
        case 't':
            return 21;
        case 'U':
        case 'u':
            return 22;
        case 'V':
        case 'v':
            return 23;
        case 'W':
        case 'w':
            return 24;
        case 'X':
        case 'x':
            return 25;
        case 'Y':
        case 'y':
            return 26;
        case 'Z':
        case 'z':
            return 27;
        default:
            return 0;
    }
}

const int jumpCharCount = 28;

int GetJumpDepth(const int depth)
{
    const int jump_stopcount = 2;
    const int jump_recursecount = 26;

    int rd = 1;
    for (int i = 0; i < depth; i++)
        rd *= jump_recursecount;
    return jump_stopcount * (1 - rd) / (1 - jump_recursecount) + rd;
}

bool IsSearchStop(const SearchIndex i)
{
    switch (i)
    {
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
            return false;
        default:
            return true;
    }
}
