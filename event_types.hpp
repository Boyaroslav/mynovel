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

#include <stdint.h>
#pragma once

struct event_type{
    uint8_t id;
    uint8_t args_count;
    const char* name;
};

#define ARG_DOUBLE 2
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
    {4, 1, "LD"}, // ${LID}
    {5, 1, "LID"},
    {6, 2, "ALIAS"},
    {7, 2, "CHSPR"}, // сменить спрайту n (начиная с нуля) картинку на name
    {8, 1, "RET"}, // -1 mainscreen -2 outofgame 
    {9, 2, "SET"},
    {10, 4, "MV"}, //1 аргумент - номер спрайта на экране с нуля. 4 аргумент - время операции для плавности
    {11, 0, "CLTB"}, // clear textbox
    {12, 4, "LDSIZE"}, // TODO сделать чтоб она текущий ищменяла + время операции добавить
    {13, 5, "LDXYWH"}, // LD with x y w and h
    {14, 1, "CALL"}, // ну пока хз но скорее всего функции буду вызывать
    {15, 3, "OPERATION"}, // операции над переменными
    {16, 1, "TBCAP"}, // change textbox footer text - name of talking character and so on
    {17, 2, "WAIT"}, // таймер
    {18, 3, "IF"}, // earg (><==) earg -> можно использовать __return == 1 коли сложная логика нужна
    {19, 2, "JMP"}, // думаю либо можно будет номер команды указать, либо метку __METKA0808 сделаю возможность создавать их по ходу пьесы 
    {20, 1, "LUA"}, // execute LUA string 
    {21, 0, "ELSE"},
    {22, 0, "ENDIF"},
    {23, 1, "BPM"}, // bg play music name - music name / 0 - pause 
    {24, 1, "SOUND"}, // play sound name - не прерывает текущие звуки, то есть лучше создать комманду talk чтоб прерывала короче
    {25, 1, "BPMFADEIN"}, // втухание типо. меняет переменную внутри класса аудио и только
    {26, 1, "LUA_IMPORT"}, // я заебался забывать формулу импорта библиотеки
    {27, 1, "LDFILE"}, // было бы недурно разделять все таки bin-ы
    {28, 2, "BPMVOL"},
    {29, 2, "SOUNDVOL"},
    {30, 2, "BPMCROSS"}, // новый трек  с угасанием и тд
    {31, 2, "SOUNDLOOP"}, // ну получается у нас может много аундов звучать, значит через нее можно цикадочек подрубить чтоб на фоне играло
    {32, 1, "SOUNDSTOP"}, // по имени зацикленный саунд остановить
    {33, 1, "BPMFADEOUT"}, // затухание типо. меняет переменную внутри класса аудио и только


    


};

const int EVENT_TYPE_COUNT = sizeof(event_types)/sizeof(event_types[0]);