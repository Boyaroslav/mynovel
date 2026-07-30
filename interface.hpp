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
    SDL_Rect cords;
    std::string text="";
    std::function<void()> event;
    public:

    Button(SDL_Rect c, std::function<void()> e, std::string l=""){
        event = e;
        cords = c;
        text = l;
    }

    bool check_hovered(int mx, int my, int x, int y){
        return mx >= cords.x+x
            && mx <= cords.x+x+cords.w
            && my >= cords.y+y
            && my <= cords.y+y+cords.h;

    }

    void draw(SDL_Renderer *rend, int xx, int yy, bool is_hovered=false){
        SDL_Rect r = {cords.x+xx, cords.y+yy, cords.w, cords.h};

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

    bool check_press(int mx, int my, int x, int y){
        bool pressed = check_hovered(mx, my, x, y);

        if(pressed) action();

        return pressed;
    }

    void action(){
        event();
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

    void add_button(Button b) {
        things.push_back(b);
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