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

#pragma once

#include "screen.hpp"


inline std::unique_ptr<Menu> make_pause_menu(int width, int height, Screen* s) {
    auto m = std::make_unique<Menu>(width/4, 30, width, height);
    m->add_button(Button({0,   0, width/2, 40}, [s]{ s->hide_interface(); }, "Resume"));
    m->add_button(Button({0,   50, width/2, 40}, [s]{ s->qsave(); s->hide_interface();}, "Quick save"));
    m->add_button(Button({0,   100, width/2, 40}, [s]{ s->qload(); s->hide_interface();}, "Quick load"));
    m->add_button(Button({0,  150, width/2, 40}, [s]{ s->open_settings(); },  "Settings"));
    m->add_button(Button({0, 200, width/2, 40}, [s]{ s->change_scene("menu"); s->main_menu(); },              "Exit"));
    return m;
}

inline std::unique_ptr<Menu> make_settings_menu(int width, int height, Screen* s) {
    auto m = std::make_unique<Menu>(width/2, 30, width/2, height);
    m->add_button(Button({0, 0, width/2, 40}, [s]{ s->hide_interface(); s->show_interface(); }, "Back"));
    return m;
}

inline std::unique_ptr<Menu> make_main_menu(int width, int height, Screen* s) {
    auto m = std::make_unique<Menu>(width/4, 30, width, height);
    m->add_button(Button({0, 0, width/2, 40}, [s]{s->start_game();}, "Start"));
    m->add_button(Button({0, 200, width/2, 40}, [s]{ s->exit_program(); }, "Quit"));
    return m;
}