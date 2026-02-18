#include "utils.hpp"
#include "event_types.hpp"
#include "scene.hpp"
#include "event.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pool.hpp"


int load_file(const char* filename, std::vector<Scene>& sc_out, int* out_count) {
    sc_out.clear();

    FILE* f = fopen(filename, "rb");
    if (!f) {
        perror("Cannot open file");
        return 0;
    }

    int scene_count;
    if (fread(&scene_count, sizeof(int), 1, f) != 1) {
        fclose(f);
        return 0;
    }

    sc_out.resize(scene_count);

    if (fread(sc_out.data(), sizeof(Scene), scene_count, f) != scene_count) {
        fclose(f);
        return 0;
    }

    int total_events = 0;
    int total_args = 0;
    int total_strings = 0;

    for (int i = 0; i < scene_count; i++) {
        total_events += sc_out[i].event_count;
        total_args += sc_out[i].string_size; // wrong — fix below
    }

    total_events = 0;
    total_args = 0;
    total_strings = 0;

    for (int i = 0; i < scene_count; i++) {
        total_events += sc_out[i].event_count;
        total_strings += sc_out[i].string_size;
    }

    std::vector<Event> evs(total_events);
    if (fread(evs.data(), sizeof(Event), total_events, f) != total_events) {
        fclose(f);
        return 0;
    }

    for (auto& e : evs) total_args += e.args_count;

    std::vector<earg> args(total_args);
    if (fread(args.data(), sizeof(earg), total_args, f) != total_args) {
        fclose(f);
        return 0;
    }

    std::vector<char> strings(total_strings);
    if (fread(strings.data(), 1, total_strings, f) != total_strings) {
        fclose(f);
        return 0;
    }

    fclose(f);

    int base_spos = spos;
    memcpy(spool + spos, strings.data(), total_strings);
    spos += total_strings;

    int base_apos = apos;
    for (auto& a : args) {
        earg t = a;
        if (t.type == ARG_STRING) {
            t.value += base_spos;
        }
        apool[apos++] = t;
    }

    int base_epos = epos;
    for (auto& e : evs) {
        Event t = e;
        t.args_offset += base_apos;
        epool[epos++] = t;
    }

    for (auto& sc : sc_out) {
        sc.event_start += base_epos;
        sc.string_start += base_spos;
    }

    if (out_count) *out_count = scene_count;
    return 1;
}


int find_scene_index_by_name(const std::vector<Scene>& scenes, const std::string& name) {
    for (size_t i = 0; i < scenes.size(); ++i) {
        if (strcmp(scenes[i].name, name.c_str()) == 0) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void list_scenes(const std::vector<Scene>& scenes){
    for(int i=0; i<scenes.size(); i++){
        std::cout<<"SCENE "<<i<<" "<<scenes[i].name<<"\n";
    }
}