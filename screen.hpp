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




#include "loader.hpp"
#include "textbox.cpp"
#include "gamemixer.hpp"
#include "luaruntime.hpp"
#include "camera.hpp"
#include <setjmp.h>

class Screen
{
private:
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    LuaRuntime lua_runtime;
    int px, py;
    int event_pool_position = 0;
    uint32_t last_time;
    std::unique_ptr<Menu> interface;
    std::unique_ptr<Camera> camera;
    bool if_result = 1;

    bool if_its_game = 1; // если это типо главное меню а не игра чтоб ESC не прожимался и тд


    int row_n = 0;
    int row_executed = 0;
    bool IN_ROW = false;

    Scene *current_scene = nullptr;
    int event_pool_position_buffer = 0;
    bool WAITING = false;
    std::string current_scene_name;
    std::string current_script_name = "";
    float wait_timer = 0.0f;
    int function_commands_left = 0; // когда вызвается функция CALL [func] то на scene->event_count количество эвентов мы берем команды с пула со сдвигом функции, потом возвращаем каретку обратно

    Sprite bg = Sprite();

    std::vector<Sprite> sprites; // персонажы и какие нибудь предметы хз
    bool NEED_MORE_EVENTS = false;
    float bg_alpha = 1.0f;
    float bg_speed = get_value("VAR_BG_CHANGE_SPEED");
    std::string current_bg_path;

    bool running = true;
    std::string var_waiting = "";
    std::vector<uint8_t> save_snapshot; // мы его каждый ход обновляем и когда save то сейвим

public:
    std::string file_name;
    std::unique_ptr<TextBox> textbox;
    std::unique_ptr<Audio> audio;
    SDL_Event e{};
    bool WAS_MOTION = false;

    Screen() = default;
    std::vector<Scene> scenes;
    int scenes_number = 0;
    Event *current_event = nullptr;
    std::function<void()>need_to_do;

    bool init_();
    void load_(char *name);
    void nextEvent();
    bool change_scene(const char *scene_name);
    void handleEvent(bool isnext_needed = true);
    void handleMouseEvent(const SDL_Event &e);
    void run(abool &run);
    void update_and_render();
    void clean();
    void save();
    void load();
    void qsave();
    void qload();
    void show_interface();
    void hide_interface();
    void open_settings();
    void start_game();
    void main_menu();
    void set_if_its_game(bool state);
    void exit_program();
    void write_states(FILE *ptr);
    void update_snapshot();
    bool can_save();
    void build_n_run_interface(std::function<std::unique_ptr<Menu>(int, int, Screen*)> b);
    SDL_Renderer *getRenderer() const;
};