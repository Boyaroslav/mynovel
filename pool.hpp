#include "utils.hpp"
#include "event.hpp"
#include "event_types.hpp"

// STRING POOL

const char ENDSTR = '\0';

char spool[STRING_POOL_SIZE];

int spos = 0;

int add_string(const char* s) {
    int start = spos;
    while (*s) spool[spos++] = *s++;
    spool[spos++] = 0;
    return start;
}

const char* get_from_spool(int index) {
    if (index < 0 || index >= spos) return nullptr;
    return &spool[index];
}

//EVENT POOL


Event epool[EVENT_POOL_SIZE];

int epos = 0;


void add_to_epool(Event s) {
    epool[epos++] = s;
}

Event* get_from_epool(int index) {
    if (index < 0 || index >= epos) return nullptr;
    return &epool[index];
}

// ARGS POOl

earg apool[ARGS_POOL_SIZE];

int apos = 0;

void add_argument(earg s) {

    apool[apos++] = s;

}

earg* get_from_apool(int index) {
    if (index < 0 || index >= apos) return nullptr;
    return &apool[index];
}


