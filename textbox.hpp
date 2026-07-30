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
#include "sprite.hpp"

Font main_font;

Font text_box_font;

// TODO - добавить id к сообщениям чтобы lua и тд могли изменять смску по id а не только последнее



// я тут подумал, active words должны быть только у поседней смски, то есть меньше считать и все такое(не забудь удалять)

struct baked_element{
    SDL_Texture* tex = nullptr;
    SDL_Rect dst;
};


struct rendered_aw{
    SDL_Rect r;
    std::string laction;

};

struct message{
    //uint8_t id=-1;
    std::string text;
    float speed; // frames per second
    std::chrono::steady_clock::time_point start_time;
    std::vector<active_words> aw;
    float chars_shown = 0.0f;
    bool is_complete = false; // true - запекаем значит
    baked_element be;
    std::vector<rendered_aw> local_aws; // их же тоже удалять с сообщением

    message(std::string text,
            float speed,
            std::chrono::steady_clock::time_point start_time,
            std::vector<active_words> aw,
            float chars_shown = 0.0f,
            bool is_complete=false): 
            text(text), speed(speed), start_time(start_time), aw(aw), chars_shown(chars_shown), is_complete(is_complete) {}

        ~message() {
        if (be.tex) SDL_DestroyTexture(be.tex);
    }

    message(const message&) = delete;
    message& operator=(const message&) = delete;


    message(message&& other) noexcept
        : text(std::move(other.text)),
          speed(other.speed),
          start_time(other.start_time),
          aw(std::move(other.aw)),
          chars_shown(other.chars_shown),
          is_complete(other.is_complete),
          be(other.be),
          local_aws(std::move(other.local_aws))
    {
        other.be.tex = nullptr;
    }

    message& operator=(message&& other) noexcept
    {
        if (this != &other) {
            if (be.tex) SDL_DestroyTexture(be.tex);
            text = std::move(other.text);
            speed = other.speed;
            start_time = other.start_time;
            aw = std::move(other.aw);
            chars_shown = other.chars_shown;
            is_complete = other.is_complete;
            be = other.be;
            local_aws = std::move(other.local_aws);
            other.be.tex = nullptr;
        }
        return *this;
    }
};


struct textbox_button{
    rendered_aw rectact;
    SDL_Texture* unpressed;
    SDL_Texture* pressed;
};

class TextBox{
    protected:
        int max_width = 0;
        SDL_Color Outline_color;
        int step = 1;
        std::string footer;
        int line_height = 1;
        SDL_Rect border;
        SDL_Rect tb_border;
        float scroll_y = 0.0f;
        float target_scroll_y = 0.0f;
        int move_x = 0;
        int move_y = 0;
        int padding = 20;
        toml::table configuration;
        bool stick_bottom=0;

        int length_of_last_message = 0;
        int length_of_last_elements = 0; // бля я чето не то делаю
        int baked_line_count = 0;


        bool IS_TOML = false;

        bool draw_frame = 1;

        bool IS_SPRITE = 0;

        Sprite tb_sprite;

        int max_lines = 4;
        SDL_Color box_color = to_sdlc(DEFAULT_BOX_COLOR);
        std::vector<message>messages; // [text angry slow] \n [text normal quick and so on]
        std::vector<rendered_aw>r_aws;
        std::chrono::steady_clock::time_point last_update;

        std::vector<std::vector<text_line>> lines;

        std::vector<baked_element> baked_lines;
    public:
        bool hidden = false;
        TextBox(){
            log("TextBox created"); hidden = false; IS_INPUT=0;
            input_header_size=0; IS_HOVERED=0;
            Outline_color =  to_sdlc(TEXTBOX_OUTLINE_COLOR);
        }
        ~TextBox(){
            for(auto& i: baked_lines){
                SDL_DestroyTexture(i.tex);
            }
            r_aws.clear(); messages.clear();}
        void addMessage(std::string);
        void cl();
        void cllast();
        void draw(SDL_Renderer*);
        void set_footer(std::string);
        void update(float);
        void hide();
        void show();
        void done_messages();
        std::string* get_last();
        void refresh_last();
        std::pair<std::vector<active_words>, std::string> parse_active_words(std::string text);
        void handle_mouse_wheel(SDL_Event e);
        void is_hovered(int px, int py); // hover - парить, зависать - типо мышка наведена ли
        bool is_last_completed();
        void update_position(int, int);
        void move_position(int, int);
        bool IS_INPUT;
        bool IS_HOVERED;
        void check_cursor(int, int);
        void check_press(int, int);
        void write_yourself(FILE* ptr);
        void read_yourself(FILE* ptr);
        void load_toml(SDL_Renderer* r, std::string t);
        bool WAS_ACTION = false; // если на слово нажали чтоб не жмалось NEED_MORE_EVENTS
        int input_header_size;
        void bake_lines();
        void bake_line();
        void clear_completed();
        void bake_completed(SDL_Renderer* rend);

        std::vector<std::vector<text_line>> split_message(message& msg);
        std::vector<std::vector<text_line>> fit_lines(message &msg, int max_width);
};