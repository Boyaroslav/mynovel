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
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <variant>
#include <unordered_map>
#include <cstdio>
#include <cstdint>
#include <chrono>
#include <SDL.h>
#include <SDL_image.h>
#include <functional>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include "lua.hpp"
#include "settings.cpp"

#define FORCE_CHARS_SHOWN_FLUENCY 0


const uint32_t VERSION = 1000;

const int STRING_POOL_SIZE = 65536;
const int EVENT_POOL_SIZE = 1024;
const int ARGS_POOL_SIZE = 4096;
const int TEXT_BOX_MESSAGES = 12;
const int TEXT_BOX_INITIAL_VERTICAL_GAP = 10;
const int TEXT_BOX_VERTICAL_GAP = 10;
const int DEFAULT_FONT_SIZE = 28;
const int TEXT_BOX_HORIZONTAL_PADDING = 100;
const int TEXT_BOX_VERTICAL_PADDING = 20;

const char* save_path = "";

const int REMOVE_LINES = 0; // remove lines when the number reaches max_lines

float LETTER_SPEED = 0.07;

int text_box_scroll_step = 30;

int INVERSED_SCROLL = 1;

#ifdef FORCE_CHARS_SHOWN_FLUENCY
float MAX_CHARS_SPEED = 1;
#endif

std::string LUA_ACTION_FROM_TEXTBOX = "";

bool IS_CCNVL = false; // если .bin то все как раньше. а если .ccnvl то будем по другому брать фотки, другие файлы там и тд

FILE* ccnvl_file = nullptr; // будем хранить открытый указатель на файл чтоб если что оттуда чего читать (пока не храним)
uint8_t* ccnvl_data  = nullptr;
uint32_t scene_data_start = 0;

// const char* DEFAULT_FONT = "FreeMono.ttf";
const char *DEFAULT_FONT = "./NotoSansMonoCJKsc-Regular.otf";

char FONT[512];

struct index_db_element{
    uint32_t offset;
    uint32_t size;

};

// в ccnvl будем хранить массив .bin чтобы по нему бегать и по названию быстренько загружать через load_scenes_from_memory

// это будет использоваться только при основном режиме работы - чтении .CCNVL
std::unordered_map<uint32_t, index_db_element>ccnvl_resources;
std::unordered_map<uint32_t, index_db_element>ccnvl_scenes;


#ifdef __ANDROID__
#include <android/log.h>
#define LOG_TAG "LCNOVEL"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#endif

void log(std::string msg)
{
    std::cout << msg << "\n";
    #ifdef __ANDROID__
    LOGI("LCNOVEL LOG %s", msg.c_str());
    #endif
    return;

}

void log(SDL_Rect msg)
{
    std::cout << "SDL_Rect: " << msg.x << ' ' << msg.y << ' ' << msg.w << ' ' << msg.h << "\n";
    return;
}

uint64_t fnv1a(const std::string &s)
{
    uint64_t hash = 1469598103934665603ULL;

    for (char c : s)
    {
        hash ^= (uint8_t)c;
        hash *= 1099511628211ULL;
    }

    return hash;
}

uint32_t fnv1a_32(const std::string &s)
{
    uint32_t hash = 2166136261u;
    for (char c : s)
    {
        hash ^= (uint8_t)c;
        hash *= 16777619u;
    }
    return hash;
}

int real_width = 1024;
int real_height = 600;
int width = real_width;
int height = real_height;

#define abool std::atomic<bool>

SDL_Color to_sdlc(Color x)
{
    SDL_Color result = {255, 255, 255, 255};

    if (x.t == 'r')
    {
        result = {x.color.r, x.color.g, x.color.b, x.color.a};
        return result;
    }

    std::string hexs = std::string(x.hex);
    uint32_t hexValue = 0;

    try
    {
        hexValue = std::stoul(hexs, nullptr, 16);
    }
    catch (...)
    {
        return result;
    }

    if (hexs.length() == 6)
    {
        result.r = (hexValue >> 16) & 0xFF;
        result.g = (hexValue >> 8) & 0xFF;
        result.b = hexValue & 0xFF;
        result.a = 255;
    }
    else if (hexs.length() == 8)
    {
        result.r = (hexValue >> 24) & 0xFF;
        result.g = (hexValue >> 16) & 0xFF;
        result.b = (hexValue >> 8) & 0xFF;
        result.a = hexValue & 0xFF;
    }
    else if (hexs.length() == 3)
    {
        result.r = ((hexValue >> 8) & 0xF) * 17;
        result.g = ((hexValue >> 4) & 0xF) * 17;
        result.b = (hexValue & 0xF) * 17;
        result.a = 255;
    }

    return result;
}

static size_t utf8_len(const std::string &s)
{
    size_t count = 0;
    for (size_t i = 0; i < s.size();)
    {
        unsigned char c = s[i];
        size_t char_len =
            (c < 0x80) ? 1 : (c < 0xE0) ? 2
                         : (c < 0xF0)   ? 3
                                        : 4;
        i += char_len;
        count++;
    }
    return count;
}

static std::string utf8_substr(const std::string &s, size_t start, size_t count)
{
    size_t i = 0, cs = 0, ce = 0;
    size_t pos = 0;

    while (i < s.size() && cs < start)
    {
        unsigned char c = s[i];
        size_t len = (c < 0x80) ? 1 : (c < 0xE0) ? 2
                                  : (c < 0xF0)   ? 3
                                                 : 4;
        i += len;
        cs++;
    }

    pos = i;

    while (i < s.size() && ce < count)
    {
        unsigned char c = s[i];
        size_t len = (c < 0x80) ? 1 : (c < 0xE0) ? 2
                                  : (c < 0xF0)   ? 3
                                                 : 4;
        i += len;
        ce++;
    }

    return s.substr(pos, i - pos);
}

bool has_extension(const char *name, const char *ext)
{
    const char *dot = strrchr(name, '.');
    if (!dot)
        return false;
    return strcmp(dot, ext) == 0;
}

// from textbox.hpp
struct active_words{
    uint32_t start;
    uint32_t end;
    std::string lua_action;
        active_words(unsigned int start_, unsigned int end_, const std::string& function_)
        : start(start_), end(end_), lua_action(function_) {}
};

struct ActiveWord{
    active_words aw;
    std::string text;
};

using text_line = std::variant<std::string, ActiveWord>;


long long int max(long long int x, long long int y){
    if (x >= y) return x;
    return y;
}

long long int min(long long int x, long long int y){
    if (x <= y){ return x;}
    return y;
}

/*

SDL_Color interface_fill = to_sdlc(INTERFACE_FILL_COLOR);
SDL_Color interface_border = to_sdlc(INTERFACE_BORDER_COLOR);
SDL_Color interface_text = to_sdlc(INTERFACE_TEXT_COLOR);
SDL_Color interface_text_border = to_sdlc(INTERFACE_TEXT_BORDER_COLOR);

*/

struct mem_buffer {
    std::vector<char> data;
    size_t pos = 0;
    off_t offset;
};


#if defined(__linux__) && !defined(__ANDROID__)
static ssize_t 
#else
static int
#endif

lcnovel_mem_write_cb(void* cookie, const char* buf,
    #if defined(__linux__) && !defined(__ANDROID__)
    size_t
    #else
    int
    #endif
    size) {
    mem_buffer* mb = static_cast<mem_buffer*>(cookie);
    mb->data.insert(mb->data.end(), buf, buf + size);
    return size;
}


static int lcnovel_mem_close_cb(void*) {
    return 0;
}

#if defined(__linux__) && !defined(__ANDROID__)
static ssize_t 
#else
static int
#endif
lcnovel_mem_read_cb(void* cookie, char* buf,
    #if defined(__linux__) && !defined(__ANDROID__)
    size_t
    #else 
    int
    #endif
    size
){

    mem_buffer* mb = static_cast<mem_buffer*>(cookie);
    if ((!mb) || (!buf) || (size == 0)) {
        return -1;
    }

    size_t bytes_to_copy = min(size, mb->data.size());
    memcpy(buf, mb->data.data() + mb->pos, bytes_to_copy);
    mb->pos += bytes_to_copy;

    return (int)bytes_to_copy;

}

static int lcnovel_mem_seek_cb(void* cookie, off_t *offset, int whence){
    off_t newoff;
    mem_buffer* mb = static_cast<mem_buffer*>(cookie);
    switch (whence)
    {
    case SEEK_SET:
        newoff = *offset;
        break;
    case SEEK_END:
        newoff = mb->data.size() + *offset;

    break;

    case SEEK_CUR:
        newoff = mb->pos + *offset;
    break;
    
    default:
        return -1;
    }

    mb->offset = newoff;
    *offset = newoff;
    return 0;

}


#if defined(__linux__) && !defined(__ANDROID__)
static FILE* lcnovel_open_mem_stream(mem_buffer* mb){
    cookie_io_functions_t mfuncs = {
        .read = lcnovel_mem_read_cb,
        .write = lcnovel_mem_write_cb,
        .seek = lcnovel_mem_seek_cb,
        .close = lcnovel_mem_close_cb

    };
    auto s = fopencookie(
        mb,
        "w+",
        mfuncs

    );
    if (s == NULL) {
        log("fopencookie error");
    }
    return s;
}
#else


static FILE* lcnovel_open_mem_stream(mem_buffer* mb) {
    return funopen(
        mb,
        nullptr,
        lcnovel_mem_write_cb,
        nullptr,
        lcnovel_mem_close_cb
    );
}

#endif
