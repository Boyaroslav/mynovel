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

int check_aw(uint32_t i, const message& m) // возвращает индекс в msg.aw
{
    int l = 0;
    int r = m.aw.size() - 1;

    while (l <= r)
    {
        int mid = l + (r - l) / 2;
        const auto& aw = m.aw[mid];

        if (i < aw.start)
            r = mid - 1;
        else if (i > aw.end)
            l = mid + 1;
        else
            return mid;
    }

    return -1;
}


std::vector<std::vector<text_line>> TextBox::split_message(message& msg)
{
    std::vector<std::vector<text_line>> result;

    std::string visible_text = msg.is_complete
        ? msg.text
        : utf8_substr(msg.text, 0, (size_t)msg.chars_shown);

    if (visible_text.empty())
        return result;

    std::vector<text_line> current_line;
    std::string word;

    size_t total_utf8_len = utf8_len(visible_text);
    for (size_t i = 0; i < total_utf8_len; ++i)
    {
        std::string c = utf8_substr(visible_text, i, 1);

        if (c != " ")
        {
            word += c;
            continue;
        }

        if (text_box_font.measure(current_line, word).x > max_width)
        {
            result.push_back(current_line);
            current_line.clear();
        }

        int aw_idx = check_aw(i - utf8_len(word), msg);
        current_line.push_back(aw_idx >= 0
            ? text_line(ActiveWord{msg.aw[aw_idx], word + " "})
            : text_line(word + " "));

        word.clear();
    }

    if (!word.empty() || !current_line.empty())
    {
        if (text_box_font.measure(current_line, word).x > max_width)
        {
            result.push_back(current_line);
            result.push_back({ word });
        }
        else
        {
            current_line.push_back(word);
            result.push_back(current_line);
        }
    }

    return result;
}


void TextBox::draw(SDL_Renderer *rend)
{
    if (hidden) return;
    if (!rend) return;

    SDL_RenderSetClipRect(rend, IS_TOML ? &tb_border : &border);
    if (IS_SPRITE){
        tb_sprite.draw(rend);
        for(auto& b: interactives){
            std::visit([&](auto& t){
                t.draw(rend, tb_border.x, tb_border.y);
            }, b);
            
        }
    }

    if (draw_frame){
        SDL_SetRenderDrawColor(rend, box_color.r, box_color.g, box_color.b, box_color.a);
        SDL_RenderFillRect(rend, &border);
    }

    r_aws.clear();
    max_width   = border.w - padding * 2;
    line_height = text_box_font.measure("A").y + 6;

    int x = border.x + padding;

    // готовим ещё не запечённое (печатающееся) последнее сообщение
    std::vector<std::vector<text_line>> current_lines;
    if (!messages.empty() && (!messages.back().is_complete || IS_INPUT || !messages.back().be.tex))
        current_lines = split_message(messages.back());

    // считаем полную высоту контента для скролла
    int total_height = 0;
    for (auto& msg : messages)
        if (msg.is_complete && msg.be.tex)
            total_height += msg.be.dst.h;
    total_height += (int)current_lines.size() * line_height;

    int visible_height = border.h - padding * 2;
    int max_scroll = max(0, total_height - visible_height);
    if (target_scroll_y > max_scroll) target_scroll_y = max_scroll;
    if (target_scroll_y < 0) target_scroll_y = 0;

    if (stick_bottom){
        target_scroll_y=max(0, total_height - visible_height);
    }

    SDL_RenderSetClipRect(rend, &border);

    int y = border.y + padding - (int)target_scroll_y;

    // рисуем уже запечённые сообщения одной текстурой каждое
    for (auto& msg : messages)
    {
        if (!msg.is_complete || !msg.be.tex) continue;

        SDL_Rect dst{ x, y, msg.be.dst.w, msg.be.dst.h };
        SDL_RenderCopy(rend, msg.be.tex, nullptr, &dst);

        for (auto& la : msg.local_aws)
        {
            SDL_Rect r = la.r;
            r.x += x;
            r.y += y;
            r_aws.push_back({ r, la.laction });
        }

        y += dst.h;
    }

    // рисуем печатающееся сообщение как раньше — по частям
    for (auto& line : current_lines)
    {
        int cur_x = x;
        for (auto& part : line)
        {
            if (std::holds_alternative<std::string>(part))
            {
                const std::string& text = std::get<std::string>(part);
                if (text.empty()) continue;

                SDL_Texture* tex = text_box_font.renderOutlined(rend, text
                    /*DEFAULT_FONT_COLOR, DEFAULT_FONT_BORDER_COLOR*/);
                if (!tex) continue;

                SDL_Point sz = text_box_font.measure(text);
                SDL_Rect dst{cur_x, y, sz.x, sz.y};
                SDL_RenderCopy(rend, tex, nullptr, &dst);
                SDL_DestroyTexture(tex);

                cur_x += sz.x;
            }

            else
            {
                const ActiveWord& aw = std::get<ActiveWord>(part);
                if (aw.text.empty()) continue;

                SDL_Texture* tex = text_box_font.renderOutlinedUnderlineBold(rend, aw.text);
                if (!tex) continue;

                SDL_Point sz = text_box_font.measure(aw.text);
                SDL_Rect dst{cur_x, y, sz.x, sz.y};
                SDL_RenderCopy(rend, tex, nullptr, &dst);
                SDL_DestroyTexture(tex);

                r_aws.push_back({ dst, aw.aw.lua_action });
                cur_x += sz.x;
            }
        }
        y += line_height;
    }

    SDL_RenderSetClipRect(rend, nullptr);

    SDL_RenderCopy(rend, baked_footer.tex, nullptr, &baked_footer.dst);
    if (IS_HOVERED && draw_frame) {
        SDL_SetRenderDrawColor(rend, Outline_color.r, Outline_color.g, Outline_color.b, Outline_color.a);
        SDL_RenderDrawRect(rend, &border);

        SDL_Rect thick_border = {border.x + 1, border.y + 1, border.w - 2, border.h - 2};
        SDL_RenderDrawRect(rend, &thick_border);
    }
}


void TextBox::set_footer(SDL_Renderer* rend, std::string t="")
{
    if (t.size()) footer = t;
    SDL_DestroyTexture(baked_footer.tex);
    baked_footer.tex = text_footer_font.renderOutlined(rend,
        footer);
}

bool TextBox::is_last_completed(){
    if (messages.empty()) return true;
    return messages.back().is_complete;
}

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
    stick_bottom = 0;
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
    length_of_last_message = 0;
    target_scroll_y = 999999;
    //log("I have new message! " + text);
    text = interpolate(text);
    if (messages.size())
        done_messages();
    auto [aw, t] = TextBox::parse_active_words(text);
    messages.emplace_back(t, 1.0 * get_value("LETTER_SPEED").as_float(), std::chrono::steady_clock::now(), aw, 0);
    std::cout<<messages.back().speed<<"\n";
    stick_bottom=1;


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
    for(auto& b: interactives){
        std::visit([&](auto& t){
            t.check_hovered(px, py, tb_border.x, tb_border.y);
        }, b);
    }
}

void TextBox::clear_completed(){
    for(auto it = messages.begin(); it != messages.end();){
        auto &msg = *it;
        if (msg.is_complete){
            messages.erase(it);
            continue;
        }


        it++;
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
    tb_border.x -= move_x;
    tb_border.y -= move_y;
    tb_border.x += x_;
    tb_border.y += y_;
    baked_footer.dst.x -= move_x - x_;
    baked_footer.dst.y -= move_y - y_;
    tb_sprite.move(x_ - move_x, y_ - move_y);
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
            return;
        }
    }

    for(auto& b : interactives){
        std::visit([&](auto& t){
            t.check_press(px, py, tb_border.x, tb_border.y);
        }, b);
    }
}

void TextBox::check_cosmetic_press(int px, int py){

    for(auto& b : interactives){
        
        std::visit([&](auto& t){
            t.check_cosmetic_press(px, py, tb_border.x, tb_border.y);
        }, b);
    }

}

void TextBox::show(){
    hidden = 0;
    set_value("TEXTBOX_HIDDEN", 0);
}

void TextBox::hide(){
    hidden = 1;
    set_value("TEXTBOX_HIDDEN", 1);
}


void TextBox::write_yourself(FILE* ptr){
    fwrite(&move_x, sizeof(uint32_t), 1, ptr);
    fwrite(&move_y, sizeof(uint32_t), 1, ptr);
    uint32_t fs = footer.size();
    fwrite(&fs, sizeof(uint32_t), 1, ptr);
    fwrite(footer.data(), sizeof(char), fs, ptr);
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
    uint32_t fs; fread(&fs, sizeof(uint32_t), 1, ptr);
    fread(footer.data(), sizeof(char), fs, ptr);

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
    log("reading toml...");

    tb_sprite.IGNORE_CAMERA = 1;

    std::string f = configuration["textarea"]["font"].value_or<std::string>(""); // если пустой то он сам найдет шрифт в .load
    text_box_font.load(f.c_str());
    text_box_font.setSize(configuration["textarea"]["font_size"].value_or(DEFAULT_FONT_SIZE));

    f = configuration["footer"]["font"].value_or<std::string>("");
    text_footer_font.load(f.c_str());
    text_footer_font.setSize(configuration["footer"]["font_size"].value_or(DEFAULT_FONT_SIZE));

    if (configuration.empty()) return;

    text_box_font.setColor(Color{configuration["textarea"]["color"].value_or<std::string>("").c_str()});
    text_box_font.setBorderColor(Color{configuration["textarea"]["outline_color"].value_or<std::string>("").c_str()});
    text_box_font.setActiveColor(Color{configuration["textarea"]["active_color"].value_or<std::string>("").c_str()});
    text_box_font.setActiveBorderColor(Color{configuration["textarea"]["active_outline"].value_or<std::string>("").c_str()});
    text_footer_font.setColor(Color{configuration["footer"]["color"].value_or<std::string>("").c_str()});
    text_footer_font.setBorderColor(Color{configuration["footer"]["outline_color"].value_or<std::string>("").c_str()});


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

    baked_footer.dst = SDL_Rect{
        configuration["footer"]["x"].value_or(0) + tb_border.x,
        configuration["footer"]["y"].value_or(0) + tb_border.y,
        configuration["footer"]["width"].value_or(0),
        configuration["footer"]["height"].value_or(0)
    };

    footer = configuration["footer"]["default"].value_or<std::string>("");

    if (footer.size()) set_footer(rend, footer);


    padding = configuration["textarea"]["padding"].value_or(20);

    std::string texture_name = configuration["textbox"]["image"].value_or("");
    
    if (texture_name.size()){
    tb_sprite.load_texture(rend, (texture_name).c_str());

    tb_sprite.set_rect(tb_border);
    IS_SPRITE = 1;
    }
    draw_frame = (bool)configuration["textbox"]["draw_frame"].value_or(0);
    SDL_Rect cc;
    auto bts =  configuration["textbox"]["button"].as_array();
    
    for(auto&& n : *bts){
        auto& but = *n.as_table();
        cc = SDL_Rect{
            but["x"].value_or(0),
            but["y"].value_or(0),
            but["width"].value_or(0),
            but["height"].value_or(0),

        };
        interactives.emplace_back(
            std::move(Button{cc, run_lua_func, but["command"].value_or("")})
        );
        std::visit([&](auto& t){
            t.make_me_image(rend, but["image"].value_or(""));
            t.add_hov_prsd(rend, but["image_hover"].value_or(""), but["image_pressed"].value_or(""));
        }, interactives.back());

    }



}

void TextBox::bake_completed(SDL_Renderer* rend)
{
    if (!rend) return;

    for (auto& msg : messages)
    {
        if ((&msg == &messages.back()) && IS_INPUT) continue;
        if (!msg.is_complete) continue;
        if (msg.be.tex != nullptr) continue;

        auto msg_lines = split_message(msg);
        if (msg_lines.empty()) continue;

        int tex_w = max_width;
        int tex_h = (int)msg_lines.size() * line_height;
        if (tex_w <= 0 || tex_h <= 0) continue;

        SDL_Texture* target = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET, tex_w, tex_h);
        if (!target) continue;

        SDL_SetTextureBlendMode(target, SDL_BLENDMODE_BLEND);

        SDL_Texture* prev_target = SDL_GetRenderTarget(rend);
        SDL_SetRenderTarget(rend, target);
        SDL_SetRenderDrawColor(rend, 0, 0, 0, 0);
        SDL_RenderClear(rend);

        int y = 0;
        for (auto& line : msg_lines)
        {
            int cur_x = 0;
            for (auto& part : line)
            {
                if (std::holds_alternative<std::string>(part))
                {
                    const std::string& text = std::get<std::string>(part);
                    if (text.empty()) continue;

                    SDL_Texture* part_tex = text_box_font.renderOutlined(rend, text
                    /*    DEFAULT_FONT_COLOR, DEFAULT_FONT_BORDER_COLOR*/);
                    if (!part_tex) continue;

                    SDL_Point sz = text_box_font.measure(text);
                    SDL_Rect dst{cur_x, y, sz.x, sz.y};
                    SDL_RenderCopy(rend, part_tex, nullptr, &dst);
                    SDL_DestroyTexture(part_tex);

                    cur_x += sz.x;
                }
                else
                {
                    const ActiveWord& aw = std::get<ActiveWord>(part);
                    if (aw.text.empty()) continue;

                    SDL_Texture* part_tex = text_box_font.renderOutlinedUnderlineBold(rend, aw.text
                    /*    ACTIVE_FONT_COLOR, DEFAULT_ACTIVE_FONT_BORDER_COLOR*/);
                    if (!part_tex) continue;

                    SDL_Point sz = text_box_font.measure(aw.text);
                    SDL_Rect dst{cur_x, y, sz.x, sz.y};
                    SDL_RenderCopy(rend, part_tex, nullptr, &dst);
                    SDL_DestroyTexture(part_tex);

                    msg.local_aws.push_back({dst, aw.aw.lua_action});

                    cur_x += sz.x;
                }
            }
            y += line_height;
        }

        SDL_SetRenderTarget(rend, prev_target);

        msg.be.tex = target;
        msg.be.dst = SDL_Rect{ 0, 0, tex_w, tex_h }; 
    }
}