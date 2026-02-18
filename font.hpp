#include "utils.hpp"

class Font
{
private:
    TTF_Font* font = nullptr;
    SDL_Color color = to_sdlc(DEFAULT_FONT_COLOR);
    SDL_Color border_color = to_sdlc(DEFAULT_FONT_BORDER_COLOR);


public:
    int size = DEFAULT_FONT_SIZE;

    Font() = default;

    bool load()
    {
        font = TTF_OpenFont(DEFAULT_FONT, size);
        TTF_SetFontOutline(font, 1);

        return font != nullptr;
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
                            Color main, Color outline,
                            int outlineSize = 2) const
{
    if (!font) return nullptr;

    SDL_Color mainColor = to_sdlc(main);
    SDL_Color outlineColor = to_sdlc(outline);
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

};