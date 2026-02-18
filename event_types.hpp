#include <stdint.h>
#pragma once

struct event_type{
    uint8_t id;
    uint8_t args_count;
    const char* name;
};

#define ARG_STRING 1
#define ARG_INT 0

struct earg{
    uint8_t type;
    uint32_t value;
};


const event_type event_types[]= {
    {1, 1, "TXT"},
    {2, 1, "ROW"}, // следующие n комманд выполняются сразу
    {3, 1, "BG"},
    {4, 1, "LD"},
    {5, 1, "LID"},
    {6, 2, "ALIAS"},
    {7, 2, "CHSPR"},
    {8, 1, "RET"}, // -1 mainscreen -2 outofgame 
    {9, 2, "SET"},
    {10, 4, "MV"},
    {11, 0, "CLTB"}, // clear textbox
    {12, 3, "LDSIZE"}, // change size of sprite in px
    {13, 5, "LDXYWH"}, // LD with x y w and h
    {14, 1, "CALL"}, // ну пока хз но скорее всего функции буду вызывать
    


};

const int EVENT_TYPE_COUNT = sizeof(event_types)/sizeof(event_types[0]);