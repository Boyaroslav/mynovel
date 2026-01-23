#pragma once

#include "utils.hpp"
#include "args.hpp"
#include "event_types.hpp"



#pragma pack(push, 1)  
struct event{
    uint8_t type;
    uint8_t arg_count;
    uint32_t args_offset;
};
#pragma pack(pop)