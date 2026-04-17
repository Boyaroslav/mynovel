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

#include "utils.hpp"

class Audio
{
private:
    Mix_Music *current_music = nullptr; // та что прям вот щас играет

    int fade_in = 0;
    int fade_out = 0;

    int sfx_fade_in = 0;
    int sfx_fade_out = 0;

    int bgm_volume = MIX_MAX_VOLUME; // max - 128
    int sfx_volume = MIX_MAX_VOLUME; // same

    std::unordered_map<uint32_t, Mix_Chunk *> chunks; // кэш чанков
    std::unordered_map<uint32_t, int> channel;        // что играет (хэш) - на каком канале

    Mix_Chunk *h;

public:
    void fade_out_music(int m) { fade_out = m; }
    void fade_in_music(int m) { fade_in = m; }

    Audio() {}

    ~Audio()
    {
        for (auto i = chunks.begin(); i != chunks.end(); i++){
            Mix_FreeChunk(i->second);
        }

        if (current_music)
        {
            Mix_HaltMusic();
            Mix_FreeMusic(current_music);
            current_music = nullptr;
        }
    }
    Mix_Chunk *get_chunk(const char *path)
    {
        uint32_t key = fnv1a_32(path);
        auto it = chunks.find(key);
        if (it != chunks.end())
            return it->second;

        Mix_Chunk *chunk = load_resource_chunk(path);
        if (chunk)
            chunks[key] = chunk;
        return chunk;
    }

    Mix_Music *load_resource_music(const char *path)
    {
        uint32_t hash = fnv1a_32(path);

        auto it = ccnvl_resources.find(hash);
        if (it == ccnvl_resources.end())
        {
            log("CCNVL resource not found: " + std::string(path));
            return nullptr;
        }

        auto &res = it->second;
        SDL_RWops *rw = SDL_RWFromMem(ccnvl_data + res.offset, res.size);
        if (!rw)
            return nullptr;

        Mix_Music *mus = Mix_LoadMUS_RW(rw, 1);
        if (!mus)
        {
            log("Failed to load music from CCNVL: " + std::string(path));
            return nullptr;
        }

        return mus;
    }

    Mix_Chunk *load_resource_chunk(const char *path)
    {
        uint32_t hash = fnv1a_32(path);

        auto it = ccnvl_resources.find(hash);
        if (it == ccnvl_resources.end())
        {
            log("CCNVL resource not found: " + std::string(path));
            return nullptr;
        }

        auto &res = it->second;
        SDL_RWops *rw = SDL_RWFromMem(ccnvl_data + res.offset, res.size);
        if (!rw)
            return nullptr;

        Mix_Chunk *mus = Mix_LoadWAV_RW(rw, 1);
        if (!mus)
        {
            log("Failed to load music from CCNVL: " + std::string(path));
            return nullptr;
        }

        return mus;
    }
    // chunk - целиком в памяти; несколько одновременно - sound
    // music - не целиком в памяти; один одновременно - bgm
    // если есть в массиве песенок то ее играем. Иначе сначала load_resource а потом играем

    bool if_sound_in_chunks(uint32_t s)
    {
        return chunks.find(s) != chunks.end();
    }

    void play_music(const char *path)
    {
        if (path == nullptr || std::string(path) == "0")
        {
            if (fade_out > 0)
                Mix_FadeOutMusic((int)fade_out);
            else
                Mix_PauseMusic();
            return;
        }
        if (current_music)
        {
            Mix_HaltMusic();
            Mix_FreeMusic(current_music);
            current_music = nullptr;
        }
        Mix_FadeOutMusic(fade_out);

        current_music = load_resource_music(path);
        Mix_VolumeMusic(bgm_volume);
        if (fade_in > 0)
            Mix_FadeInMusic(current_music, -1, (int)fade_in);
        else
            Mix_PlayMusic(current_music, -1);
    }

    void play_audio(const char *path, bool stop_previous=0, int spec_volume=-1)
    {
        if(stop_previous){
            log("stop previous!");
            Mix_HaltChannel(-1);
            
        }
        log("SOUND " + std::string(path));
        uint32_t hash = fnv1a_32(path);
        int ch;
        if (if_sound_in_chunks(hash))
        {
            log(std::string(path) + " IS ALREADY IN CHUNKS");
            Mix_Chunk *i = get_chunk(path);
            
            ch = Mix_PlayChannel(-1, i, 0);
            Mix_Volume(ch, (spec_volume == -1) ? sfx_volume : spec_volume);
        }
        else{
            Mix_Chunk* i = load_resource_chunk(path);
            uint32_t hash = fnv1a_32(path);
            if (i == nullptr){return;}
            chunks[hash] = i;
            ch = Mix_PlayChannel(-1, i, 0);
            Mix_Volume(ch, (spec_volume == -1) ? sfx_volume : spec_volume);
        }
    }

    void stop_music() {}
    void set_music_volume(int v){
        bgm_volume = (int)(((double)MIX_MAX_VOLUME / 100.0) * (double)v);
        Mix_VolumeMusic(bgm_volume);
    }
    void set_sound_volume(const char* path, int v){
        uint32_t hash = fnv1a_32(path);

        sfx_volume = (int)(((double)MIX_MAX_VOLUME / 100.0) * (double)v);
        Mix_Volume(-1, sfx_volume);
    }
};