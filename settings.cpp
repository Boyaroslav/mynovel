/*
 * This file is part of CnCn (mynovel).
 * Copyright (C) 2026 Iaroslav Bobylev
 * CnCn (mynovel) is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * CnCn (mynovel) is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with CnCn (mynovel). If not, see <https://www.gnu.org/licenses/>.
 */


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

Color TEXTBOX_OUTLINE_COLOR = {
    rgbac{255, 255, 255, 50}
};

Color DEFAULT_FONT_BORDER_COLOR = {rgbac{0,0,0,255}};
