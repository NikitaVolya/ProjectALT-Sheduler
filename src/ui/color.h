#ifndef _COLOR_H_
#define _COLOR_H_

#include <SDL2/SDL.h>

typedef struct {
    Uint8 r, g, b, a;
} ColorRGBA;

ColorRGBA create_colorRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

Uint8 get_colorRGBA_red(ColorRGBA c);

Uint8 get_colorRGBA_green(ColorRGBA c);

Uint8 get_colorRGBA_blue(ColorRGBA c);

Uint8 get_colorRGBA_alpha(ColorRGBA c);

#endif /* _COLOR_H_ */