#include"color.h"

ColorRGBA create_colorRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    ColorRGBA res;

    res.r = r;
    res.g = g;
    res.b = b;
    res.a = a;

    return res;
}

Uint8 get_colorRGBA_red(ColorRGBA c) {
    return c.r;
}

Uint8 get_colorRGBA_green(ColorRGBA c) {
    return c.g;
}

Uint8 get_colorRGBA_blue(ColorRGBA c) {
    return c.b;
}

Uint8 get_colorRGBA_alpha(ColorRGBA c) {
    return c.a;
}
