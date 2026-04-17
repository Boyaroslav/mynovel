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

void TextBox::draw(SDL_Renderer *rend)
{
    SDL_SetRenderDrawColor(rend, box_color.r, box_color.g, box_color.b, box_color.a);
    SDL_RenderFillRect(rend, &border);

    int padding = 20;
    int max_width = border.w - padding * 2;
    std::vector<std::string> lines;

    for (auto &msg : messages)
    {
        std::string visible_text = utf8_substr(msg.text, 0, (size_t)msg.chars_shown);
        if (visible_text.empty())
            continue;

        std::string current_line = "";
        std::string word = "";

        size_t total_utf8_len = utf8_len(visible_text);
        for (size_t i = 0; i < total_utf8_len; ++i)
        {
            std::string c = utf8_substr(visible_text, i, 1);

            if (c == " ")
            {
                SDL_Point sz = main_font.measure(current_line + word);
                if (sz.x > max_width)
                {
                    lines.push_back(current_line);
                    current_line = word + " ";
                }
                else
                {
                    current_line += word + " ";
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
            SDL_Point sz = main_font.measure(current_line + word);
            if (sz.x > max_width)
            {
                lines.push_back(current_line);
                lines.push_back(word);
            }
            else
            {
                lines.push_back(current_line + word);
            }
        }
    }

    // высота строки
    line_height = main_font.measure("A").y + 6;

    // если строк слишком много — оставляем только последние (как в чате)
    if ((int)lines.size() > max_lines)
    {
        lines.erase(lines.begin(), lines.end() - max_lines);
    }

    // стартовый Y — всегда фиксированный
    int y = border.y + padding;
    int x = border.x + padding;

    // рендерим строки
    for (auto &line : lines)
    {
        SDL_Texture *tex = main_font.renderOutlined(rend, line, DEFAULT_FONT_COLOR, DEFAULT_FONT_BORDER_COLOR);
        // SDL_Texture* tex = main_font.render(rend, line);
        if (!tex)
            continue;

        SDL_Point sz = main_font.measure(line);
        SDL_Rect dst{x, y, sz.x, sz.y};
        SDL_RenderCopy(rend, tex, nullptr, &dst);
        SDL_DestroyTexture(tex);

        y += line_height;
    }
    if (IS_HOVERED) {
        
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
    int start_x = e.wheel.x;
    int start_y = e.wheel.y;


}

void TextBox::addMessage(std::string text)
{
    if (messages.size())
        done_messages();
    messages.emplace_back(interpolate(text), 1.0 * (variables["LETTER_SPEED"]).as_int() / 1000.0, std::chrono::steady_clock::now(), 0);
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
    border.x = TEXT_BOX_HORIZONTAL_PADDING;
    border.w = w - TEXT_BOX_HORIZONTAL_PADDING * 2;
    border.h = h / 3;
    border.y = h - border.h - TEXT_BOX_VERTICAL_PADDING;
    max_lines = border.h / line_height;
}