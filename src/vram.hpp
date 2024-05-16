#include <fxcg/display.h>

#ifndef VRAM_HPP
#define VRAM_HPP

// extern color_t *const VRAM;

extern color_t *VRAM;
void Setup_VRAM();

#define VRAMpos(x, y) VRAM[x + y * LCD_WIDTH_PX]

#endif
