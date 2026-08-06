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



#include "font.hpp"

extern Font main_font;

class Button{
    private:
    bool IMG = 0;
    bool IHP = 0;
    bool pressed = 0;
    bool hovered = 0;
    bool cosmetic_pressed=0;
    SDL_Rect cords;
    SDL_Rect src;
    SDL_Texture* tex = nullptr;
    SDL_Texture* tex_pressed = nullptr;
    SDL_Texture* tex_hovered = nullptr;
    SDL_Surface* surf = nullptr; // она понадобится чтоб коллайдер смотреть
    SDL_Texture* collidetex = nullptr; // если кнопка сложной формы
    std::function<void()> event;
    std::function<void(std::string&)> sevent; // хз как было бы лучше 
    public:
    std::string text="";

    Button(SDL_Rect c, std::function<void()> e, std::string l=""){
        event = e;
        cords = c;
        text = l;
    }

    Button(SDL_Rect c, std::function<void(std::string&)> e, std::string l){
        sevent = e;
        cords = c;
        text = l;

    }

    Button(){
        
    }

    ~Button(){
        SDL_FreeSurface(surf);
        SDL_DestroyTexture(tex);
        SDL_DestroyTexture(collidetex);

    }

    void make_me_image(SDL_Renderer* rend, const char* path){ // обязательно вызвать конструктор до этого

        uint32_t hash = fnv1a_32(path);

        auto it = ccnvl_resources.find(hash);
        if (it == ccnvl_resources.end()){
            log("Button: cannot load " + std::string(path));
            return;
        }
        auto& res = it->second;

        SDL_RWops* rw = SDL_RWFromMem(ccnvl_data + res.offset, res.size);

        if (!rw) return;

        surf = IMG_Load_RW(rw, 1);

        if (!surf){
            log("Button: surface not loading");
            return;
        }

        tex = SDL_CreateTextureFromSurface(rend, surf);
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
        SDL_QueryTexture(tex, NULL, NULL, &src.w, &src.h);
        src.x = 0;
        src.y = 0;


        IMG = 1;


    }

    void add_hov_prsd(SDL_Renderer* rend, const char* hov, const char* prd){
        if (hov){
        uint32_t hash = fnv1a_32(hov);

        auto it = ccnvl_resources.find(hash);
        if (it == ccnvl_resources.end()){
            log("Button: cannot load " + std::string(hov));
            return;
        }
        auto& res = it->second;

        SDL_RWops* rw = SDL_RWFromMem(ccnvl_data + res.offset, res.size);
        auto _surf = IMG_Load_RW(rw, 1);

        if (!_surf){
            log("Button: surface not loading");
            return;
        }

        tex_hovered = SDL_CreateTextureFromSurface(rend, _surf);
        SDL_SetTextureBlendMode(tex_hovered, SDL_BLENDMODE_BLEND);
        }

        if (prd){

        uint32_t hash = fnv1a_32(prd);
        auto it = ccnvl_resources.find(hash);
        if (it == ccnvl_resources.end()){
            log("Button: cannot load " + std::string(prd));
            return;
        }
        auto& res = it->second;
        auto rw  = SDL_RWFromMem(ccnvl_data + res.offset, res.size);
        auto _surf = IMG_Load_RW(rw, 1);
        if (!_surf){
            log("Button: surface not loading");
            return;
        }
        tex_pressed = SDL_CreateTextureFromSurface(rend, _surf);
        SDL_SetTextureBlendMode(tex_pressed, SDL_BLENDMODE_BLEND);
        IHP = 1; // лан похуй
        }

    }

    void give_me_collider(const char* collider){ // collider это типо коллизия

    }

    Button& operator=(const Button&) = delete;

    Button& operator=(Button&&) noexcept = default;

    Button(const Button& other){
        cords = other.cords;
        text = other.text;
        event = other.event;
        sevent = other.sevent;

        // ресурсы не копируем
        IMG = false;
        tex = nullptr;
        surf = nullptr;
        collidetex = nullptr;
    }

    Button(Button&& other) noexcept{ // move
        cords = other.cords;
        text = std::move(other.text);
        src = other.src;

        tex = other.tex;
        tex_hovered = other.tex_hovered;
        tex_pressed = other.tex_pressed;
        surf = other.surf;
        collidetex = other.collidetex;
        IMG = other.IMG;
        IHP = other.IHP;

        event = std::move(other.event);
        sevent = std::move(other.sevent);

        other.tex = nullptr;
        other.surf = nullptr;
        other.collidetex = nullptr;
        other.tex_hovered = nullptr;
        other.tex_pressed = nullptr;
    }

    bool check_hovered(int mx, int my, int x, int y){
    
        hovered = IMG? if_pixel(surf, mx - x - cords.x, my - y - cords.y, cords.w, cords.h) : mx >= cords.x+x
            && mx <= cords.x+x+cords.w
            && my >= cords.y+y
            && my <= cords.y+y+cords.h;

        return hovered;

    }

    void draw(SDL_Renderer *rend, int xx, int yy, bool is_hovered=false){
        SDL_Rect r = {cords.x+xx, cords.y+yy, cords.w, cords.h};

        if (IMG){
            if (IHP){
                if (cosmetic_pressed){SDL_RenderCopy(rend, tex_pressed, nullptr, &r); return;}
                if (hovered){SDL_RenderCopy(rend, tex_hovered, nullptr, &r); return;}
            }
            SDL_RenderCopy(rend, tex, nullptr, &r);
            return;
        }

        Color fill = is_hovered ? HOVERED_INTERFACE_FILL_COLOR : INTERFACE_FILL_COLOR;
        SDL_Color fc = to_sdlc(fill);
        SDL_SetRenderDrawColor(rend, fc.r, fc.g, fc.b, fc.a);
        SDL_RenderFillRect(rend, &r);

        Color border = is_hovered ? HOVERED_INTERFACE_BORDER_COLOR : INTERFACE_BORDER_COLOR;
        SDL_Color bc = to_sdlc(border);
        SDL_SetRenderDrawColor(rend, bc.r, bc.g, bc.b, bc.a);
        SDL_RenderDrawRect(rend, &r);

        if(!text.empty()){
            SDL_Texture* tex = main_font.renderOutlined(rend, text,
                INTERFACE_TEXT_COLOR, INTERFACE_TEXT_BORDER_COLOR, 2);
            if(tex){
                SDL_Point sz = main_font.measure(text);
                SDL_Rect tr = {r.x+(r.w-sz.x)/2, r.y+(r.h-sz.y)/2, sz.x, sz.y};
                SDL_RenderCopy(rend, tex, nullptr, &tr);
                SDL_DestroyTexture(tex);
            }
        }
    }
    bool check_cosmetic_press(int mx, int my, int x, int y){
        cosmetic_pressed = check_hovered(mx, my, x, y);
        return pressed;
    }

    bool check_press(int mx, int my, int x, int y){
        cosmetic_pressed =0;
        pressed = check_hovered(mx, my, x, y);

        if(pressed) action();

        return pressed;
    }

    void action(){
        event? event() :  sevent(text);
    }

};

using MENU_THINGS = std::variant<Button>;

class Menu{
    private:
    std::vector<MENU_THINGS>things; // ImageButton, Widget, Slider вся шелуха короче будет
    int x, y, w, h;
    bool if_shown = 0;
    public:
    bool smth_pressed=0;

    Menu() = default;

    Menu(int x, int y, int w, int h, std::initializer_list<MENU_THINGS> items = {})
     : x(x), y(y), w(w), h(h), things(items) {}

     

    void draw(SDL_Renderer *rend, int mouse_x=-1000, int mouse_y = -1000){
        if(if_shown){
        for(int i=0; i<things.size(); i++){
            std::visit([&](auto& thing) {
                bool hov = thing.check_hovered(mouse_x, mouse_y, x, y);
                thing.draw(rend, x, y, hov);
            }, things[i]);
        }
        }
    }

    void add_button(Button&& b) {
        things.emplace_back(std::move(b));
    }

    bool shown(){
        return if_shown;
    }

    void hide(){
        if_shown = 0;
    }
    void show(){
        if_shown = 1;
    }

    void handle_click(int mouse_x, int mouse_y) {
    if (!if_shown) return;
    bool pr = 0;
    for(int i=0; i<things.size(); i++){
        std::visit([&](auto& thing) {
            if(thing.check_press(mouse_x, mouse_y, x, y))
                pr=1;
        }, things[i]);
    }
    smth_pressed=pr;
}
};