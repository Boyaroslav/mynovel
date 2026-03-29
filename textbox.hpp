/*
 * This file is part of CCN (mynovel).
 * Copyright (C) 2026 Iaroslav Bobylev
 * CCN (mynovel) is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * CCN (mynovel) is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with CCN (mynovel). If not, see <https://www.gnu.org/licenses/>.
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
        int step = 1;
        std::string footer;
        SDL_Rect border = {100, 400, 500, 600};
        SDL_Color box_color = to_sdlc(DEFAULT_BOX_COLOR);
        std::vector<message>messages; // [text angry slow] \n [text normal quick and so on]
        std::chrono::steady_clock::time_point last_update;
    public:

        void addMessage(std::string);
        void cl();
        void cllast();
        void draw(SDL_Renderer*);
        void set_footer(std::string);
        void update(float);
        void done_messages();
        void update_position(int, int);
};