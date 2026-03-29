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

#include "gameapp.hpp"
#include "scene.hpp"

int main(int argc, char *argv[])
{

    char n[32] = "cock novel";

    abool run = true;

    const char *env = getenv("CCNVL_FILE");

    const char *font_ = getenv("FONT");


    if (env)
    {
        strncpy(n, env, sizeof(n) - 1);
        n[sizeof(n) - 1] = 0;
    }
    else
    {
        strcpy(n, "test2.ccnvl");
    }
    if(font_){
        strcpy(FONT, font_);
    }
    else{
        strcpy(FONT, DEFAULT_FONT);
    }
    FONT[sizeof(FONT)-2] = '\n';

    Screen scr = Screen();
    Scene scene;
    scr.init_();

    scr.load_(n);
    scr.change_scene("main");

    scr.run(run);

    scr.clean();

    return 0;
}