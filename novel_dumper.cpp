#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "pool.hpp"
#include "scene.hpp"
#include "event_types.hpp"

bool is_number(const char* s) {
    if (!*s) return false;
    if (*s == '-' || *s == '+') s++;
    while (*s) {
        if (*s < '0' || *s > '9') return false;
        s++;
    }
    return true;
}

int parse_args(char* line, char** argv, int max_args) {
    int argc = 0;
    bool in_quotes = false;
    char* start = nullptr;

    for (char* p = line; ; ++p) {
        char c = *p;

        if (c == '"' && !in_quotes) {
            in_quotes = true;
            start = p + 1;
        } else if (c == '"' && in_quotes) {
            *p = 0;
            argv[argc++] = start;
            in_quotes = false;
            start = nullptr;
            if (argc >= max_args) break;
        } else if ((c == ' ' || c == 0) && !in_quotes) {
            if (start) {
                *p = 0;
                argv[argc++] = start;
                start = nullptr;
                if (argc >= max_args) break;
            }
            if (c == 0) break;
        } else {
            if (!start && c != ' ') start = p;
        }

        if (c == 0) break;
    }

    return argc;
}

void add_event(uint8_t id, int argc, char** argv) {
    if (epos >= EVENT_POOL_SIZE) return;
    if (apos + argc >= ARGS_POOL_SIZE) return;

    Event e;
    e.id = id;
    e.args_count = argc;
    e.args_offset = apos;

    for (int i = 0; i < argc; ++i) {
        if (is_number(argv[i])) {
            apool[apos].type = ARG_INT;
            apool[apos].value = atoi(argv[i]);
        } else {
            int si = add_string(argv[i]);
            apool[apos].type = ARG_STRING;
            apool[apos].value = si;
        }
        apos++;
    }

    epool[epos++] = e;
}

int main() {
    char line[512];
    Scene scenes[64]; // максимум 64 скрипта
    int scene_count = 0;

    while (fgets(line, sizeof(line), stdin)) {
        line[strcspn(line, "\r\n")] = 0;
        if (line[0] == '/' && line[1] == '/') continue;
        if (line[0] == 0) continue;

        if (!strcmp(line, "DONE")) break;
        if ((strncmp(line, "SCRIPT ", 7) == 0) || (strncmp(line, "FUNC ", 5) == 0)) {
            if (scene_count >= 64) break;

            Scene& sc = scenes[scene_count++];
            memset(&sc, 0, sizeof(Scene));
            char* name;

            if (strncmp(line, "SCRIPT ", 7) == 0) name = line + 7;
            else name = line + 5;
            int len = strlen(name);
            if (len > 31) len = 31;
            strncpy(sc.name, name, len);
            sc.name[len] = 0;

            sc.id = scene_count;
            sc.event_start = epos;
            sc.string_start = spos;

            while (fgets(line, sizeof(line), stdin)) {
                line[strcspn(line, "\r\n")] = 0;
                if (!strcmp(line, "END")) break;

                char* argv[8];
                int argc = 0;

                char* cmd = strtok(line, " ");
                if (!cmd) continue;

                uint8_t id = 0;
                for (int i = 0; i < EVENT_TYPE_COUNT; ++i) {
                    if (!strcmp(cmd, event_types[i].name)) {
                        id = event_types[i].id;
                        break;
                    }
                }
                if (!id) continue;

                char* rest = strtok(nullptr, "");
                if (rest) argc = parse_args(rest, argv, 8);

                add_event(id, argc, argv);
            }

            sc.event_count = epos - sc.event_start;
            sc.string_size = spos - sc.string_start;
        }
    }

    FILE* f = fopen("scene.bin", "wb");
    if (!f) return 1;

    // Сначала количество сцен
    fwrite(&scene_count, sizeof(int), 1, f);
    // Потом структуры сцен
    fwrite(scenes, sizeof(Scene), scene_count, f);
    // События
    fwrite(epool, sizeof(Event), epos, f);
    // Аргументы
    fwrite(apool, sizeof(earg), apos, f);
    // Строки
    fwrite(spool, 1, spos, f);

    fclose(f);
    return 0;
}
