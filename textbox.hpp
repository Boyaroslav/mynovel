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
#include "font.hpp"

Font main_font;

struct message{
    std::string text;
    float speed; // frames per second
    std::chrono::steady_clock::time_point start_time;
    float chars_shown = 0.0f;
    bool is_complete = false;
};

class TextBox{
    protected:
        SDL_Color Outline_color;
        int step = 1;
        std::string footer;
        int line_height;
        SDL_Rect border;
        int max_lines = 4;
        SDL_Color box_color = to_sdlc(DEFAULT_BOX_COLOR);
        std::vector<message>messages; // [text angry slow] \n [text normal quick and so on]
        std::chrono::steady_clock::time_point last_update;
    public:
        TextBox(){IS_INPUT=0; input_header_size=0; IS_HOVERED=0; Outline_color =  to_sdlc(TEXTBOX_OUTLINE_COLOR);}

        void addMessage(std::string);
        void cl();
        void cllast();
        void draw(SDL_Renderer*);
        void set_footer(std::string);
        void update(float);
        void done_messages();
        std::string* get_last();
        void refresh_last();
        void handle_mouse_wheel(SDL_Event e);
        void is_hovered(int px, int py); // hover - парить, зависать - типо мышка наведена ли
        bool is_last_completed();
        void update_position(int, int);
        bool IS_INPUT;
        bool IS_HOVERED;
        int input_header_size;
};