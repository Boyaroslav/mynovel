#pragma once


enum ArgType : uint8_t {
    ARG_INT = 0,
    ARG_FLOAT = 1,
    ARG_STRING = 2,
    ARG_BOOL = 3
};

#pragma pack(push, 1)  
struct arg{
    uint8_t t;
    union {
        int32_t i;       // int
        float f;         // float
        uint32_t offset; // string offset в pool
    } val;
    uint32_t len;  // for string only

};
#pragma pack(pop)