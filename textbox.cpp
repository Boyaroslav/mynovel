/*
 * This file is part of CnCn (mynovel).
 * Copyright (C) 2026 Iaroslav Bobylev
 * CnCn (mynovel) is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * CnCn(mynovel) is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with CnCn (mynovel). If not, see <https://www.gnu.org/licenses/>.
 */

#include "textbox.hpp"
#include "vars.hpp"

int check_aw(uint32_t i, message m){ // active - length; no - -1
    for (auto aw: m.aw){
        if (i >= aw.start && i <= aw.end){
            return aw.end - i;
        }
    }
    return -1;
} // я это буду использовать чтобы проверять слово на активность



void TextBox::draw(SDL_Renderer *rend)
{
    if(hidden) return;
    if (!rend) return;

    
    SDL_RenderSetClipRect(rend, IS_TOML?  &tb_border : &border);
    if (IS_SPRITE) tb_sprite.draw(rend);

    if (draw_frame){
    SDL_SetRenderDrawColor(rend, box_color.r, box_color.g, box_color.b, box_color.a);
    SDL_RenderFillRect(rend, &border);
    }

    r_aws.clear();
    int max_width = border.w - padding * 2;
    std::vector<std::vector<text_line>> lines;

    for (auto &msg : messages)
    {
        std::string visible_text = utf8_substr(msg.text, 0, (size_t)msg.chars_shown);
        if (visible_text.empty())
            continue;

        std::vector<text_line> current_line;
        std::string word = "";

        size_t total_utf8_len = utf8_len(visible_text);
        for (size_t i = 0; i < total_utf8_len; ++i)
        {
            std::string c = utf8_substr(visible_text, i, 1);

            if (c == " ")
            {
                if (text_box_font.measure(current_line, word).x > max_width)
                {
                    lines.push_back(current_line);
                    current_line.clear();
                }

                int aw_idx = check_aw(i - utf8_len(word), msg);
                if (aw_idx >= 0) {
                    for (auto &a : msg.aw) {
                        if ((i - utf8_len(word)) >= a.start && (i - utf8_len(word)) <= a.end) {
                            current_line.push_back(ActiveWord{a, word + " "});
                            break;
                        }
                    }
                } else {
                    current_line.push_back(word + " ");
                }
                word = "";
            }
            else
            {
                word += c;
            }
        }

        if (!word.empty() || !current_line.empty())
        {
            if (text_box_font.measure(current_line, word).x > max_width)
            {
                lines.push_back(current_line);
                lines.push_back({ word });
            }
            else
            {
                current_line.push_back(word);
                lines.push_back(current_line);
            }
        }
    }

    // высота строки
    line_height = text_box_font.measure("A").y + 6;

    // если строк слишком много — оставляем только последние (как в чате)
    if (((int)lines.size() > max_lines) && REMOVE_LINES)
    {
        lines.erase(lines.begin(), lines.end() - max_lines);
    }

    // стартовый Y — всегда фиксированный
    int y = border.y + padding;
    int x = border.x + padding;

    int visible_height = border.h - padding * 2;
    int total_height = lines.size() * line_height;
    int max_scroll = max(0, total_height - visible_height);
    if (target_scroll_y > max_scroll)
        target_scroll_y = max_scroll;
    if (target_scroll_y < 0)
        target_scroll_y = 0;


    // рендерим строки
    SDL_RenderSetClipRect(rend, &border);
    for (auto &line : lines)
    {
        int cur_x = x;
        for (auto &part: line){
            if (std::holds_alternative<std::string>(part)){
                const std::string &text = std::get<std::string>(part);
                if (text.empty()) continue;

                SDL_Texture *tex = text_box_font.renderOutlined(rend, text,
                    DEFAULT_FONT_COLOR, DEFAULT_FONT_BORDER_COLOR);
                if (!tex) continue;

                SDL_Point sz = text_box_font.measure(text);
                SDL_Rect dst{cur_x, y -(int)target_scroll_y, sz.x, sz.y};
                SDL_RenderCopy(rend, tex, nullptr, &dst);
                SDL_DestroyTexture(tex);

                cur_x += sz.x;
            }
            else{ // active word
            const ActiveWord &aw = std::get<ActiveWord>(part);
            if (aw.text.empty()) continue;

            SDL_Texture *tex = text_box_font.renderOutlinedUnderlineBold(rend, aw.text,
                ACTIVE_FONT_COLOR, DEFAULT_ACTIVE_FONT_BORDER_COLOR);
            if (!tex) continue;

            SDL_Point sz = text_box_font.measure(aw.text);
            SDL_Rect dst{cur_x, y - (int)target_scroll_y, sz.x, sz.y};
            SDL_RenderCopy(rend, tex, nullptr, &dst);
            SDL_DestroyTexture(tex);

            r_aws.push_back({dst, aw.aw.lua_action});

            cur_x += sz.x;

            }
    }
    y += line_height;
    }
    SDL_RenderSetClipRect(rend, nullptr);
    if (IS_HOVERED && draw_frame) {
        
        SDL_SetRenderDrawColor(rend, Outline_color.r, Outline_color.g, Outline_color.b, Outline_color.a);
        SDL_RenderDrawRect(rend, &border);

        SDL_Rect thick_border = {border.x + 1, border.y + 1, border.w - 2, border.h - 2};
        SDL_RenderDrawRect(rend, &thick_border);
    }

}

void TextBox::set_footer(std::string t)
{
    footer = t;
}

bool TextBox::is_last_completed(){return messages.size()==1 ? messages[messages.size()-1].is_complete : 1;}

void TextBox::done_messages()
{
    messages[messages.size() - 1].is_complete = true;
    messages[messages.size() - 1].chars_shown = messages[messages.size() - 1].text.size();
}

void TextBox::update(float delta_time)
{
    if (messages.empty())
        return;

    message &current_msg = messages.back();

    size_t total_chars = utf8_len(current_msg.text);

    if (!current_msg.is_complete)
    {
        float addition = (current_msg.speed / delta_time) * step;
        #ifdef FORCE_CHARS_SHOWN_FLUENCY
        if (addition > MAX_CHARS_SPEED){addition = MAX_CHARS_SPEED;}
        #endif
        current_msg.chars_shown += addition;

        if (current_msg.chars_shown >= total_chars)
        {
            current_msg.chars_shown = (float)total_chars;
            current_msg.is_complete = true;
        }

    }
}


void TextBox::refresh_last()
{
    message *last_m = &(messages[messages.size() - 1]);
    last_m->chars_shown = last_m->text.size();
    return;
}

void TextBox::handle_mouse_wheel(SDL_Event e){
    if (hidden) return;
    int start_x = 0;
    int start_y = 0;
    if (e.type == SDL_FINGERMOTION){
        start_x = e.tfinger.dx;
        start_y = (e.tfinger.dy * height);

    }
    else{
        start_x = e.wheel.x;
        start_y = e.wheel.y;
    }

    target_scroll_y -= start_y * text_box_scroll_step * INVERSED_SCROLL;
    
    if (target_scroll_y < 0)
        target_scroll_y = 0;



}

void TextBox::addMessage(std::string text)
{
    target_scroll_y = 999999;
    //log("I have new message! " + text);
    text = interpolate(text);
    if (messages.size())
        done_messages();
    auto [aw, t] = TextBox::parse_active_words(text);
    messages.emplace_back(t, 1.0 * get_value("LETTER_SPEED").as_float(), std::chrono::steady_clock::now(), aw, 0);


}

void TextBox::cl()
{
    messages.clear();
}

void TextBox::cllast()
{
    messages.pop_back();
}

void TextBox::is_hovered(int px, int py){
    IS_HOVERED = 0;
    if (px > border.x && px < border.x + border.w){
        if (py > border.y && py < border.y + border.h){
            IS_HOVERED = 1;
        }
    }
}

std::string *TextBox::get_last()
{
    if (messages.size() == 0)
    {
        return nullptr;
    }
    return &(messages.back().text);
}

void TextBox::update_position(int w, int h)
{
    if (IS_TOML) return; // там по харду задаются размеры
    border.x = move_x + TEXT_BOX_HORIZONTAL_PADDING;
    border.w = w - TEXT_BOX_HORIZONTAL_PADDING * 2;
    border.h = h / 3;
    border.y = move_y + h - border.h - TEXT_BOX_VERTICAL_PADDING;
    max_lines = border.h / line_height;

}

void TextBox::move_position(int x_, int y_){
    border.x -= move_x;
    border.x += x_;
    border.y -= move_y;
    border.y += y_;
    move_x = x_;
    move_y = y_;
}


std::pair<std::vector<active_words>, std::string> TextBox::parse_active_words(std::string text) {
    std::vector<active_words> aw;
    size_t last_one = 0;
    size_t i = text.find("{{", last_one);
    while (i != std::string::npos) {
        size_t sep    = text.find("|",  i);
        size_t ending = text.find("}}", i);
        if (sep == std::string::npos || ending == std::string::npos || sep > ending)
            break;
        std::string word = text.substr(i + 2, sep - (i + 2));
        std::string func = text.substr(sep + 1, ending - (sep + 1));
        uint32_t start_char = utf8_len(text.substr(0, i));
        uint32_t end_char   = start_char + utf8_len(word) - 1;
        aw.emplace_back(start_char, end_char, func);
        text = text.replace(i, ending + 2 - i, word);
        last_one = i + word.size();
        i = text.find("{{", last_one);
    }

    return {aw, text};
}


void TextBox::check_cursor(int px, int py) {
    if (hidden) return;
    for (auto &r : r_aws) {
        if (px >= r.r.x && px <= r.r.x + r.r.w &&
            py >= r.r.y && py <= r.r.y + r.r.h)
        {
            //SDL_SetCursor(cursor_hand);
            return;
        }
    }
   // SDL_SetCursor(cursor_default);
}


void TextBox::check_press(int px, int py){
    SDL_Point p = {px, py};
    for (auto r : r_aws) {
        if (SDL_PointInRect(&p, &r.r)) {
            WAS_ACTION = 1;
            LUA_ACTION_FROM_TEXTBOX = r.laction;
        }
    }
}

void TextBox::show(){
    hidden = 0;
}

void TextBox::hide(){
    hidden = 1;
}


void TextBox::write_yourself(FILE* ptr){
    fwrite(&move_x, sizeof(uint32_t), 1, ptr);
    fwrite(&move_y, sizeof(uint32_t), 1, ptr);
    uint32_t n = messages.size();
    fwrite(&n, sizeof(uint32_t), 1, ptr);
    for(message& m : messages){
        uint32_t sz = m.text.size();
        fwrite(&sz, sizeof(uint32_t), 1, ptr);
        fwrite(m.text.data(), sizeof(char), m.text.size(), ptr);
        fwrite(&m.speed, sizeof(float), 1, ptr);
        float ch_sw = m.chars_shown;
        fwrite(&ch_sw, sizeof(float), 1, ptr);
        bool is_complete = m.is_complete;
        fwrite(&is_complete, sizeof(bool), 1, ptr);
        sz = m.aw.size();
        fwrite(&sz, sizeof(uint32_t), 1, ptr);
        for(auto& a: m.aw){
            uint32_t st = a.start;
            uint32_t en = a.end;
            uint32_t ln = a.lua_action.size();
            fwrite(&st, sizeof(uint32_t), 1, ptr);
            fwrite(&en, sizeof(uint32_t), 1, ptr);
            fwrite(&ln, sizeof(uint32_t), 1, ptr);
            fwrite(a.lua_action.data(), sizeof(char), a.lua_action.size(), ptr);

        }
    }

}

void TextBox::read_yourself(FILE* ptr){
    messages.clear();
    fread(&move_x, sizeof(uint32_t), 1, ptr);
    fread(&move_y, sizeof(uint32_t), 1, ptr);

    uint32_t n;
    fread(&n, sizeof(uint32_t), 1, ptr);

    for (uint32_t i = 0; i < n; ++i) {
        uint32_t sz;
        fread(&sz, sizeof(uint32_t), 1, ptr);

        std::string text(sz, '\0');
        fread(&text[0], sizeof(char), sz, ptr);

        float speed;
        fread(&speed, sizeof(float), 1, ptr);

        float chars_shown;
        fread(&chars_shown, sizeof(float), 1, ptr);

        bool is_complete;
        fread(&is_complete, sizeof(bool), 1, ptr);

        uint32_t aw_count;
        fread(&aw_count, sizeof(uint32_t), 1, ptr);

        std::vector<active_words> aw;
        aw.reserve(aw_count);
        for (uint32_t j = 0; j < aw_count; ++j) {
            uint32_t st, en, ln;
            fread(&st, sizeof(uint32_t), 1, ptr);
            fread(&en, sizeof(uint32_t), 1, ptr);
            fread(&ln, sizeof(uint32_t), 1, ptr);

            std::string lua_action(ln, '\0');
            fread(&lua_action[0], sizeof(char), ln, ptr);

            aw.emplace_back(st, en, lua_action);
        }

        // start_time восстанавливаем, а не читаем из файла
        float elapsed_seconds = (speed > 0.0f) ? (chars_shown / speed) : 0.0f;
        auto start_time = std::chrono::steady_clock::now()
            - std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                  std::chrono::duration<float>(elapsed_seconds));
        messages.emplace_back(text, speed, start_time, aw, chars_shown, is_complete);
    }
}


void TextBox::load_toml(SDL_Renderer* rend, std::string t){
    configuration = read_toml("textbox.toml");
    if (configuration.empty()) return;

    IS_TOML = 1;

    border = { 
        configuration["textarea"]["x"].value_or(0) + (width - configuration["textarea"]["width"].value_or(0)) / 2,
        height - configuration["textarea"]["height"].value_or(0) - configuration["textarea"]["y"].value_or(0),
        configuration["textarea"]["width"].value_or(0),
        configuration["textarea"]["height"].value_or(0)
    }; // бордер это для текста и рамки. для спрайта боксы -  tb_border

    tb_border = {
        configuration["textbox"]["x"].value_or(0) + (width - configuration["textbox"]["width"].value_or(0)) / 2,
        height - configuration["textbox"]["height"].value_or(0) - configuration["textbox"]["y"].value_or(0),
        configuration["textbox"]["width"].value_or(0),
        configuration["textbox"]["height"].value_or(0)
    };

    padding = configuration["textarea"]["padding"].value_or(20);

    std::string texture_name = configuration["textbox"]["image"].value_or("");
    
    if (texture_name.size()){
    tb_sprite.load_texture(rend, (texture_name).c_str());

    tb_sprite.set_rect(tb_border);
    IS_SPRITE = 1;
    }
    draw_frame = (bool)configuration["textbox"]["draw_frame"].value_or(0);
    std::string f = configuration["textarea"]["font"].value_or<std::string>(DEFAULT_FONT);
    text_box_font.load(f.c_str());

}