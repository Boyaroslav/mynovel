/*
 * This file is part of CCN (mynovel).
 * Copyright (C) 2026 Iaroslav Bobylev
 * CCN (mynovel) is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * CCN (mynovel) is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with CCN (mynovel). If not, see <https://www.gnu.org/licenses/>.
 */



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


