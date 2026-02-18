#pragma once
#include <stdint.h>
#include <cstring>

constexpr const char* FONT_GLOBAL = "FreeMono.ttf";
struct rgbac {
    uint8_t r,g,b,a;
};

struct Color {
    char t;
    union {
        rgbac color;
        char hex[8];
    };
    Color(const rgbac& c) : t('r') {
        color = c;
    }

    Color(const char* h) : t('h') {
        if (h) {
            strncpy(hex, h, 7);
            hex[7] = '\0';
        } else {
            strcpy(hex, "0000ff");
        }
        color = rgbac{255,0,0,255};
    }

};

Color DEFAULT_BOX_COLOR = {
    rgbac{50,50,50,80}
};

Color DEFAULT_FONT_COLOR = {
    rgbac{255,255,255,255}
};

Color DEFAULT_FONT_BORDER_COLOR = {rgbac{0,0,0,255}};
