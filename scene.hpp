#include "utils.hpp"
#include "event.hpp"
#include <cstring>
#pragma once


#pragma pack(push, 1)
struct Scene{
    uint16_t id;

    uint32_t event_start;
    uint16_t event_count;

    uint32_t string_start;
    uint32_t string_size;

    char name[32];



};
#pragma pack(pop)