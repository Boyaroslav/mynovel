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

#include "gameapp.hpp"
#include "scene.hpp"

int main(int argc, char *argv[])
{
    Screen scr = Screen();
    char n[32] = "cock novel";
    Scene scene;

    abool run = true;

    const char *env = getenv("CCNVL_FILE");

    scr.init_();

    if (env)
    {
        strncpy(n, env, sizeof(n) - 1);
        n[sizeof(n) - 1] = 0;
    }
    else
    {
        strcpy(n, "scene.bin");
    }

    scr.load_(n);
    scr.change_scene("main");

    scr.run(run);

    scr.clean();

    return 0;
}