#include <fxcg/display.h>
#include "vram.hpp"

// extern color_t *const VRAM = (color_t *)GetVRAMAddress();

color_t *VRAM;
void Setup_VRAM()
{
    VRAM = (color_t *)GetVRAMAddress();
}
