#include "utils.hpp"
#include "event_types.hpp"
#include "scene.hpp"
#include "event.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pool.hpp"

int load_bin_from_ccnvl(FILE *, uint32_t, uint32_t, std::vector<Scene> &);

int load_scenes_from_memory(const uint8_t, size_t, std::vector<Scene> &);

int load_ccnvl(const char *filename, std::vector<Scene> &sc_out, int *out_count)
{
    FILE *f = fopen(filename, "rb");
    if (!f)
    {
        perror("Cannot open CCNVL file");
        return 0;
    }

    char magic[6] = {0};
    fread(magic, 1, 5, f);

    if (strncmp(magic, "CCNVL", 5) != 0)
    {
        printf("Invalid CCNVL file\n");
        fclose(f);
        return 0;
    }

    uint8_t version;
    fread(&version, 1, 1, f);

    printf("CCNVL version %d loaded\n", version);

    uint32_t start_scene_hash;

    fread(&start_scene_hash, sizeof(uint32_t), 1, f);

    uint8_t index_db_size;
    fread(&index_db_size, sizeof(uint32_t), 1, f);
    printf("index db size - %d\n", index_db_size);
    uint32_t hash_n;
    uint32_t indx;
    uint32_t size;

    for (int i = 0; i < index_db_size; i++)
    {
        fread(&hash_n, sizeof(uint32_t), 1, f);
        fread(&indx, sizeof(uint32_t), 1, f);
        fread(&size, sizeof(uint32_t), 1, f);
        ccnvl_resources[hash_n] = index_db_element{indx, size};
    }

    sc_out.clear();


    uint32_t scene_size;
    fread(&scene_size, sizeof(uint32_t), 1, f);

    for( int i=0; i< scene_size; i++ ){
        fread(&hash_n, sizeof(uint32_t), 1, f);
        fread(&indx, sizeof(uint32_t), 1, f);
        fread(&size, sizeof(uint32_t), 1, f);
        ccnvl_scenes[hash_n] = index_db_element{indx, size};
    }

    load_bin_from_ccnvl(f, ccnvl_scenes[start_scene_hash].offset, ccnvl_scenes[start_scene_hash].size, sc_out);

    fclose(f);

    return 1;
}




int load_file(const char *filename, std::vector<Scene> &sc_out, int *out_count)
{

    if (has_extension(filename, ".ccnvl"))
    {
        IS_CCNVL = true;
        return load_ccnvl(filename, sc_out, out_count);
    }

    IS_CCNVL = false;
    sc_out.clear();

    FILE *f = fopen(filename, "rb");
    if (!f)
    {
        perror("Cannot open file");
        return 0;
    }

    int scene_count;
    if (fread(&scene_count, sizeof(int), 1, f) != 1)
    {
        fclose(f);
        return 0;
    }

    sc_out.resize(scene_count);

    if (fread(sc_out.data(), sizeof(Scene), scene_count, f) != scene_count)
    {
        fclose(f);
        return 0;
    }

    int total_events = 0;
    int total_args = 0;
    int total_strings = 0;

    for (int i = 0; i < scene_count; i++)
    {
        total_events += sc_out[i].event_count;
        total_args += sc_out[i].string_size; // wrong — fix below
    }

    total_events = 0;
    total_args = 0;
    total_strings = 0;

    for (int i = 0; i < scene_count; i++)
    {
        total_events += sc_out[i].event_count;
        total_strings += sc_out[i].string_size;
    }

    std::vector<Event> evs(total_events);
    if (fread(evs.data(), sizeof(Event), total_events, f) != total_events)
    {
        fclose(f);
        return 0;
    }

    for (auto &e : evs)
        total_args += e.args_count;

    std::vector<earg> args(total_args);
    if (fread(args.data(), sizeof(earg), total_args, f) != total_args)
    {
        fclose(f);
        return 0;
    }

    std::vector<char> strings(total_strings);
    if (fread(strings.data(), 1, total_strings, f) != total_strings)
    {
        fclose(f);
        return 0;
    }

    fclose(f);

    int base_spos = spos;
    memcpy(spool + spos, strings.data(), total_strings);
    spos += total_strings;

    int base_apos = apos;
    for (auto &a : args)
    {
        earg t = a;
        if (t.type == ARG_STRING)
        {
            t.value += base_spos;
        }
        apool[apos++] = t;
    }

    int base_epos = epos;
    for (auto &e : evs)
    {
        Event t = e;
        t.args_offset += base_apos;
        epool[epos++] = t;
    }

    for (auto &sc : sc_out)
    {
        sc.event_start += base_epos;
        sc.string_start += base_spos;
    }

    if (out_count)
        *out_count = scene_count;
    return 1;
}

int find_scene_index_by_name(const std::vector<Scene> &scenes, const std::string &name)
{
    for (size_t i = 0; i < scenes.size(); i++)
    {
        std::cout << scenes[i].name << " " << name.c_str() << "\n";
        if (strcmp(scenes[i].name, name.c_str()) == 0)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void list_scenes(const std::vector<Scene> &scenes)
{
    for (int i = 0; i < scenes.size(); i++)
    {
        std::cout << "SCENE " << i << " " << scenes[i].name << "\n";
    }
}

int load_scenes_from_memory(const uint8_t *data, size_t size,
                            std::vector<Scene> &sc_out)
{
    const uint8_t *ptr = data;
    const uint8_t *end = data + size;

    auto read = [&](void *dst, size_t sz) -> bool
    {
        if (ptr + sz > end)
            return false;

        memcpy(dst, ptr, sz);
        ptr += sz;
        return true;
    };

    int scene_count;

    if (!read(&scene_count, sizeof(int)))
        return 0;

    std::vector<Scene> scenes(scene_count);

    if (!read(scenes.data(), sizeof(Scene) * scene_count))
        return 0;

    int total_events = 0;
    int total_args = 0;
    int total_strings = 0;

    for (int i = 0; i < scene_count; i++)
    {
        total_events += scenes[i].event_count;
        total_strings += scenes[i].string_size;
    }

    std::vector<Event> evs(total_events);

    if (!read(evs.data(), sizeof(Event) * total_events))
        return 0;

    for (auto &e : evs)
        total_args += e.args_count;

    std::vector<earg> args(total_args);

    if (!read(args.data(), sizeof(earg) * total_args))
        return 0;

    std::vector<char> strings(total_strings);

    if (!read(strings.data(), total_strings))
        return 0;

    /*
        ===== теперь вставляем в глобальные пулы =====
    */

    int base_spos = spos;
    memcpy(spool + spos, strings.data(), total_strings);
    spos += total_strings;

    int base_apos = apos;

    for (auto &a : args)
    {
        earg t = a;

        if (t.type == ARG_STRING)
            t.value += base_spos;

        apool[apos++] = t;
    }

    int base_epos = epos;

    for (auto &e : evs)
    {
        Event t = e;
        t.args_offset += base_apos;
        epool[epos++] = t;
    }

    for (auto &sc : scenes)
    {
        sc.event_start += base_epos;
        sc.string_start += base_spos;

        sc_out.push_back(sc);
    }

    return 1;
}

int load_bin_from_ccnvl(FILE *f, uint32_t offset, uint32_t size,
                        std::vector<Scene> &sc_out)
{
    std::vector<uint8_t> buffer(size);

    fseek(f, offset, SEEK_SET);

    if (fread(buffer.data(), 1, size, f) != size)
        return 0;

    return load_scenes_from_memory(buffer.data(), size, sc_out);
}

int load_scene_from_ccnvl(FILE *f, uint32_t hash,
                          std::vector<Scene> &sc_out)
{
    auto it = ccnvl_scenes.find(hash);

    if (it == ccnvl_scenes.end())
    {
        printf("Scene not found\n");
        return 0;
    }

    auto &sc = it->second;

    std::vector<uint8_t> buffer(sc.size);

    fseek(f, sc.offset, SEEK_SET);

    if (fread(buffer.data(), 1, sc.size, f) != sc.size)
        return 0;

    return load_scenes_from_memory(buffer.data(), sc.size, sc_out);
}