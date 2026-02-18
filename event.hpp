#include <stdint.h>
#pragma once

struct Event{
    uint8_t id;
    uint8_t args_count;
    uint16_t args_offset;
};
