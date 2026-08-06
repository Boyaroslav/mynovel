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

#include "utils.hpp"

class Font
{
private:
    TTF_Font* font = nullptr;
    SDL_Color color = to_sdlc(DEFAULT_FONT_COLOR);
    SDL_Color border_color = to_sdlc(DEFAULT_FONT_BORDER_COLOR);

    SDL_Color active_color = to_sdlc(ACTIVE_FONT_COLOR);
    SDL_Color active_border_color = to_sdlc(DEFAULT_ACTIVE_FONT_BORDER_COLOR);


public:
    int size = DEFAULT_FONT_SIZE;

    Font() = default;

    bool load(const char* f = "")
    {
        if (!f || f[0] == '\0'){
        const char* fonts_to_try[] = {
            FONT,
            DEFAULT_FONT,
            "/system/fonts/Roboto-Regular.ttf", 

            "/usr/share/fonts/dejavu/DejaVuSansMono.ttf",

            "C:/Windows/Fonts/arial.ttf",

            "/System/Library/Fonts/Supplemental/Arial.ttf"
        };
        for (const char* path : fonts_to_try)
        {
            if (!path) continue;

            font = TTF_OpenFont(path, size);

            if (font)
            {
                log("Loaded font: " + std::string(path));
                TTF_SetFontOutline(font, 1);
                return true;
            }
        }
        }
        else{
            font = TTF_OpenFont(f, size);
            if (font)
            {
                TTF_SetFontOutline(font, 1);
                return true;
            }
        }

        log(std::string("ERROR: Could not load any font. SDL_ttf: ")
            + TTF_GetError());

        return false;
    }

    void setSize(int n){
        size = n;
        load();

    }

    SDL_Texture* render(SDL_Renderer* rend, const std::string& text) const
    {
        if (!font) return nullptr;
        TTF_SetFontOutline(font, 0);
        SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text.c_str(), color);
        if (!surf) return nullptr;
        SDL_SetSurfaceBlendMode(surf, SDL_BLENDMODE_BLEND);

        SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surf);
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
        SDL_FreeSurface(surf);
        return tex;
    }


 SDL_Texture* renderOutlined(SDL_Renderer* rend,
                            const std::string& text,
                            Color main = {}, Color outline = {},
                            int outlineSize = 2) const
{
    if (!font) return nullptr;
    SDL_Color mainColor;
    SDL_Color outlineColor;
    if (!main.is_set) mainColor = color;
    else mainColor = to_sdlc(main);
    if (!outline.is_set) outlineColor = border_color;
    else outlineColor = to_sdlc(outline);

    TTF_SetFontOutline(font, 0);
    SDL_Surface* surfMain = TTF_RenderUTF8_Blended(font, text.c_str(), mainColor);
    SDL_SetSurfaceBlendMode(surfMain, SDL_BLENDMODE_BLEND);

    TTF_SetFontOutline(font, outlineSize);
    SDL_Surface* surfOutline = TTF_RenderUTF8_Blended(font, text.c_str(), outlineColor);
    if (!surfOutline) return nullptr;
    SDL_SetSurfaceBlendMode(surfOutline, SDL_BLENDMODE_BLEND);
    if (!surfMain) {
        SDL_FreeSurface(surfOutline);
        return nullptr;
    }

    SDL_Rect dst;
    dst.x = (surfOutline->w - surfMain->w) / 2;
    dst.y = (surfOutline->h - surfMain->h) / 2;
    dst.w = surfMain->w;
    dst.h = surfMain->h;

    SDL_BlitSurface(surfMain, nullptr, surfOutline, &dst);
    SDL_FreeSurface(surfMain);

    SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surfOutline);
    SDL_FreeSurface(surfOutline);

    return tex;
}



    SDL_Point measure(const std::string& text) const
    {
        SDL_Point p{0,0};
        if (font) TTF_SizeUTF8(font, text.c_str(), &p.x, &p.y);
        return p;
    }

    TTF_Font* get()
    {
        return font;
    }

    ~Font()
    {
        if (font) TTF_CloseFont(font);
    }

    void setColor(Color c)
    {
        color = to_sdlc(c);
    }

    void setBorderColor(Color c)
    {
        border_color = to_sdlc(c);
    }

    void setActiveColor(Color c){
        active_color = to_sdlc(c);
    }
    void setActiveBorderColor(Color c){
        active_border_color = to_sdlc(c);
    }
    

    SDL_Texture* renderOutlinedUnderlineBold(SDL_Renderer* rend, const std::string& text, Color main = {}, Color outline = {}, int outlineSize = 2)
    {
        if (!font) return nullptr;

        // Включаем стиль
        TTF_SetFontStyle(font, TTF_STYLE_UNDERLINE | TTF_STYLE_BOLD);

        SDL_Color mainColor;
        SDL_Color outlineColor;
        if (!main.is_set) mainColor = active_color;
        else mainColor = to_sdlc(main);
        if (!outline.is_set) outlineColor = active_border_color;
        else outlineColor = to_sdlc(outline);

        TTF_SetFontOutline(font, 0);
        SDL_Surface* surfMain = TTF_RenderUTF8_Blended(font, text.c_str(), mainColor);
        SDL_SetSurfaceBlendMode(surfMain, SDL_BLENDMODE_BLEND);

        TTF_SetFontOutline(font, outlineSize);
        SDL_Surface* surfOutline = TTF_RenderUTF8_Blended(font, text.c_str(), outlineColor);

        // Сбрасываем стиль ДО любого return — иначе шрифт останется жирным
        TTF_SetFontStyle(font, TTF_STYLE_NORMAL);

        if (!surfOutline) return nullptr;
        SDL_SetSurfaceBlendMode(surfOutline, SDL_BLENDMODE_BLEND);

        if (!surfMain) {
            SDL_FreeSurface(surfOutline);
            return nullptr;
        }

        SDL_Rect dst;
        dst.x = (surfOutline->w - surfMain->w) / 2;
        dst.y = (surfOutline->h - surfMain->h) / 2;
        dst.w = surfMain->w;
        dst.h = surfMain->h;

        SDL_BlitSurface(surfMain, nullptr, surfOutline, &dst);
        SDL_FreeSurface(surfMain);

        SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surfOutline);
        SDL_FreeSurface(surfOutline);

        return tex;
    }

    SDL_Point measure(const std::vector<text_line>& line) {
        int total_w = 0;
        int max_h = 0;

        for (const auto& segment : line) {
            std::string text;

            // Извлекаем текст из варианта
            if (std::holds_alternative<std::string>(segment)) {
                text = std::get<std::string>(segment);
            } else {
                text = std::get<ActiveWord>(segment).text;
            }

            if (text.empty()) continue;

            SDL_Point sz = measure(text); // уже существующий measure(string)
            total_w += sz.x;
            if (sz.y > max_h) max_h = sz.y;
        }

        return {total_w, max_h};
    }

    SDL_Point measure(const std::vector<text_line>& line, const std::string& extra) {
        SDL_Point base = measure(line);
        SDL_Point ext  = measure(extra);
        return {base.x + ext.x, std::max(base.y, ext.y)};
    }

};



