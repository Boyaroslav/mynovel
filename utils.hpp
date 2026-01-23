#pragma once
#include<iostream>
#include<string>
#include<vector>
#include <atomic>
#include <unordered_map>
#include <cstdint>


void log(std::string msg){
    std::cout<<msg<<"\n";
    return;
}

int width = 800;
int height = 600;

#define str std::string
#define vecev std::vector<event>
#define vecsc std::vector<Scene>
#define abool std::atomic<bool>

#pragma pack(push, 1)
struct novel_header {
    char magic[4];  // "NVL\0"
    uint32_t version;
    uint32_t scene_count;
    uint32_t string_pool_size;
    uint32_t string_pool_offset;
    uint32_t scenes_offset;
};
#pragma pack(pop)