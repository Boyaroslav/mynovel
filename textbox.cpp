

#include "textbox.hpp"


void TextBox::draw(SDL_Renderer* rend){
    SDL_SetRenderDrawColor(rend, box_color.r, box_color.g, box_color.b, box_color.a);
    SDL_RenderFillRect(rend, &border);

    int padding = 20;
    int max_width = border.w - padding * 2;
    std::vector<std::string> lines;
    for (auto& msg : messages)
    {
        std::string visible_text = utf8_substr(msg.text, 0, (size_t)msg.chars_shown);

        if (visible_text.empty()) continue;

        size_t start = 0;
        while (start < visible_text.size()) {
            size_t len = 1;
            while (len + start <= visible_text.size()) {
                std::string substr = utf8_substr(visible_text, start, len);
                SDL_Point sz = main_font.measure(substr);
                if (sz.x > max_width) break;
                len++;
            }
            len = std::max(size_t(1), len - 1);
            lines.push_back(visible_text.substr(start, len));
            start += len;
        }
    }

// высота строки
int line_height = main_font.measure("A").y + 6;

// сколько строк помещается
int max_lines = 4;

// если строк слишком много — оставляем только последние (как в чате)
if ((int)lines.size() > max_lines) {
    lines.erase(lines.begin(), lines.end() - max_lines);
}

// стартовый Y — всегда фиксированный
int y = border.y + padding;
int x = border.x + padding;

// рендерим строки
for (auto& line : lines) {
    SDL_Texture* tex = main_font.renderOutlined(rend, line, DEFAULT_FONT_COLOR, DEFAULT_FONT_BORDER_COLOR);
    //SDL_Texture* tex = main_font.render(rend, line);
   if (!tex) continue;

    SDL_Point sz = main_font.measure(line);
    SDL_Rect dst { x, y, sz.x, sz.y };
    SDL_RenderCopy(rend, tex, nullptr, &dst);
    SDL_DestroyTexture(tex);

    y += line_height;
}

}



void TextBox::done_messages(){
    messages[messages.size()-1].is_complete = true;
    messages[messages.size()-1].chars_shown = messages[messages.size()-1].text.size();
}

void TextBox::update(float delta_time) {
    if (messages.empty()) return;

    message& current_msg = messages.back();

    size_t total_chars = utf8_len(current_msg.text);

    if (!current_msg.is_complete) {
        current_msg.chars_shown += (current_msg.speed / delta_time) * step;

        if (current_msg.chars_shown >= total_chars) {
            current_msg.chars_shown = (float)total_chars;
            current_msg.is_complete = true;
        }
    }
}


void TextBox::addMessage(std::string text){
    if(messages.size()) done_messages();
    messages.push_back({interpolate(text), 0.1, std::chrono::steady_clock::now(), 0});
}

void TextBox::cl(){
    messages.clear();
}


void TextBox::update_position(int w, int h) {
    border.x = TEXT_BOX_HORIZONTAL_PADDING;
    border.w = w - TEXT_BOX_HORIZONTAL_PADDING * 2;
    border.h = h / 3;
    border.y = h - border.h - TEXT_BOX_VERTICAL_PADDING;

}