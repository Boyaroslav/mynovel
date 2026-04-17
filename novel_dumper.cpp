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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "pool.hpp"
#include "scene.hpp"
#include "event_types.hpp"

#ifdef _WIN32
#undef main 
#endif


bool DEBUG = 1;

bool is_number(const char *s)
{
    if (!*s)
        return false;
    if (*s == '-' || *s == '+')
        s++;
    while (*s)
    {
        if (*s < '0' || *s > '9')
            return false;
        s++;
    }
    return true;
}

int parse_args(char *line, char **argv, int max_args)
{
    int argc = 0;
    bool in_quotes = false;
    char *start = nullptr;

    for (char *p = line;; ++p)
    {
        char c = *p;

        if (c == '"' && !in_quotes)
        {
            in_quotes = true;
            start = p + 1;
        }
        else if (c == '"' && in_quotes)
        {
            *p = 0;
            argv[argc++] = start;
            in_quotes = false;
            start = nullptr;
            if (argc >= max_args)
                break;
        }
        else if ((c == ' ' || c == 0) && !in_quotes)
        {
            if (start)
            {
                *p = 0;
                argv[argc++] = start;
                start = nullptr;
                if (argc >= max_args)
                    break;
            }
            if (c == 0)
                break;
        }
        else
        {
            if (!start && c != ' ')
                start = p;
        }

        if (c == 0)
            break;
    }

    return argc;
}

void add_event(uint8_t id, int argc, char **argv)
{
    if (epos >= EVENT_POOL_SIZE)
        return;
    if (apos + argc >= ARGS_POOL_SIZE)
        return;

    Event e;
    e.id = id;
    e.args_count = argc;
    e.args_offset = apos;

    for (int i = 0; i < argc; ++i)
    {
        if (is_number(argv[i]))
        {
            apool[apos].type = ARG_INT;
            apool[apos].value = atoi(argv[i]);
        }
        else
        {
            int si = add_string(argv[i]);
            apool[apos].type = ARG_STRING;
            apool[apos].value = si;
        }
        apos++;
    }

    epool[epos++] = e;
}

int main(int argc, char **argv)
{
    FILE *in = stdin;
    FILE *out = stdout;

    bool show = false;

    if (argc >= 2)
    {
        if (!strcmp(argv[1], "-showresult"))
        {
            show = true;
        }
        else
        {
            in = fopen(argv[1], "rb");
            if (!in)
            {
                fprintf(stderr, "cannot open input file\n");
                return 1;
            }
        }
    }

    if (argc >= 3)
    {
        out = fopen(argv[2], "wb");
        if (!out)
        {
            fprintf(stderr, "cannot open output file\n");
            return 1;
        }
    }

    char line[2048];
    Scene scenes[128];
    int scene_count = 0;

    while (fgets(line, sizeof(line), in))
    {

        line[strcspn(line, "\r\n")] = 0;

        if (line[0] == '/' && line[1] == '/')
            continue;
        if (line[0] == 0)
            continue;

        if (!strcmp(line, "DONE"))
            break;

        if ((strncmp(line, "SCRIPT ", 7) == 0) || (strncmp(line, "FUNC ", 5) == 0))
        {
            if (scene_count >= 64)
                break;

            Scene &sc = scenes[scene_count++];
            memset(&sc, 0, sizeof(Scene));

            char *name;

            if (strncmp(line, "SCRIPT ", 7) == 0)
                name = line + 7;
            else
                name = line + 5;

            int len = strlen(name);
            if (len > 31)
                len = 31;

            strncpy(sc.name, name, len);
            sc.name[len] = 0;

            sc.id = scene_count;
            sc.event_start = epos;
            sc.string_start = spos;

            while (fgets(line, sizeof(line), in))
            {

                line[strcspn(line, "\r\n")] = 0;

                if (!strcmp(line, "END"))
                    break;

                char *argvv[8];
                int argcv = 0;

                char *cmd = strtok(line, " ");
                if (!cmd)
                    continue;

                uint8_t id = 0;

                for (int i = 0; i < EVENT_TYPE_COUNT; ++i)
                {
                    if (!strcmp(cmd, event_types[i].name))
                    {
                        id = event_types[i].id;
                        break;
                    }
                }

                if (!id)
                    continue;

                char *rest = strtok(nullptr, "");

                if (rest)
                    argcv = parse_args(rest, argvv, 8);

                add_event(id, argcv, argvv);
            }

            sc.event_count = epos - sc.event_start;
            sc.string_size = spos - sc.string_start;
        }
    }

    fwrite(&scene_count, sizeof(int), 1, out);
    fwrite(scenes, sizeof(Scene), scene_count, out);
    fwrite(epool, sizeof(Event), epos, out);
    fwrite(apool, sizeof(earg), apos, out);
    fwrite(spool, 1, spos, out);

    if (show)
    {
        fprintf(stderr, "Scenes: %d\n", scene_count);
        fprintf(stderr, "Events: %d\n", epos);
        fprintf(stderr, "Args: %d\n", apos);
        fprintf(stderr, "Strings: %d\n", spos);
    }

    if (in != stdin)
        fclose(in);
    if (out != stdout)
        fclose(out);

    return 0;
}