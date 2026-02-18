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
        SDL_Rect border = {100, 400, 500, 600};
        SDL_Color box_color = to_sdlc(DEFAULT_BOX_COLOR);
        std::vector<message>messages; // [text angry slow] \n [text normal quick and so on]
        std::chrono::steady_clock::time_point last_update;
    public:

        void addMessage(std::string);
        void cl();
        void draw(SDL_Renderer*);
        void update(float);
        void done_messages();
        void update_position(int, int);
};