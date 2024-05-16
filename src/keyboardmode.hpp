#include <fxcg/keyboard.h>
#include <fxcg/system.h>

#ifndef KEYBOARDMODE_HPP
#define KEYBOARDMODE_HPP

const int SETUP_SETTING_KEYBOARD_MODE = 20;
// #define GetInputMode() GetSetupSetting(SETUP_SETTING_KEYBOARD_MODE)
// #define SetInputMode(mode) SetSetupSetting(SETUP_SETTING_KEYBOARD_MODE, mode)
inline unsigned char GetInputMode() { return GetSetupSetting(SETUP_SETTING_KEYBOARD_MODE); }
inline void SetInputMode(unsigned char mode) { SetSetupSetting(SETUP_SETTING_KEYBOARD_MODE, mode); }

enum KEYBOARD_MODE : unsigned char
{
    KEYBOARD_MODE_SHIFT = 0x01,
    KEYBOARD_MODE_CLIP = 0x02,
    KEYBOARD_MODE_ALPHA = 0x04,
    KEYBOARD_MODE_ALPHA_LOWER = 0x08,
    KEYBOARD_MODE_LOCK_BIT = 0x80,
    KEYBOARD_MODE_ALPHA_LOCK = 0x84,
    KEYBOARD_MODE_ALPHA_LOWER_LOCK = 0x88,
};

#endif
