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

#include "utils.hpp"
#include "event_types.hpp"
#include <variant>
#pragma once

uint32_t last_id = -1;

struct Var
{
    enum class Type
    {
        Int,
        Float,
        String,
        Null
    } type = Type::Null;

    uint32_t i = 0;
    double f = 0.0;
    std::string s;

    // удобные геттеры
    uint32_t as_int() const { return i; }
    double as_float() const { return f; }
    std::string as_string() const { return s; }

    bool is_null() const { return type == Type::Null; }
    bool is_int() const { return type == Type::Int; }
    bool is_float() const { return type == Type::Float; }
    bool is_string() const { return type == Type::String; }
    operator uint32_t() const { return i; }
    operator double() const { return f; }
    operator float() const { return (float)f; }
    operator std::string() const { return s; }
    operator bool() const { return type != Type::Null; }
    operator const char *() const { return s.c_str(); }
};

// фабричные функции
Var make_var(uint32_t v)
{
    Var r;
    r.type = Var::Type::Int;
    r.i = v;
    return r;
}
Var make_var(double v)
{
    Var r;
    r.type = Var::Type::Float;
    r.f = v;
    return r;
}
Var make_var(int v)
{
    Var r;
    r.type = Var::Type::Int;
    r.i = v;
    return r;
}
Var make_var(std::string v)
{
    Var r;
    r.type = Var::Type::String;
    r.s = v;
    return r;
}
// может где то переопределяться, чекни потом
std::unordered_map<std::string, Var> variables;

void set_value(const std::string &t, uint32_t v) { variables[t] = make_var(v); }
void set_value(const std::string &t, int v) { variables[t] = make_var(v); }
void set_value(const std::string &t, double v) { variables[t] = make_var(v); }
void set_value(const std::string &t, std::string v) { variables[t] = make_var(v); }

Var get_value(const std::string &t)
{
    auto it = variables.find(t);
    if (it != variables.end())
        return it->second;
    return Var{}; // Null
}

void vars_init()
{
    log("vars_init");
    set_value("VAR_BG_CHANGE_SPEED", 2.0);
    set_value("__running__", (uint32_t)1);
    set_value("width", width);
    set_value("height", height);
    set_value("LETTER_SPEED", LETTER_SPEED);
    set_value("LD_SPEED", 0.4); // как быстро будет выплывать спрайт
    set_value("WINDOW_TITLE", std::string("my novel"));
    set_value("TEXTBOX_HIDDEN", 0);
}

std::string interpolate(const std::string &text)
{
    std::string result;
    result.reserve(text.size());

    size_t i = 0;
    while (i < text.size())
    {
        if (text[i] == '$' && i + 1 < text.size() && text[i + 1] == '{')
        {
            size_t start = i + 2;
            size_t end = text.find('}', start);

            if (end == std::string::npos)
            {
                // незакрытая скобка — вставляем как есть
                result += text[i];
                i++;
                continue;
            }

            std::string var_name = text.substr(start, end - start);
            if (var_name == "LID")
            {
                result += std::to_string(last_id);
            }
            else
            {
                Var val = get_value(var_name);

                if (!val.is_null())
                {
                    if (val.is_int())
                        result += std::to_string(val.as_int());
                    else if (val.is_float())
                        result += std::to_string(val.as_float());
                    else if (val.is_string())
                        result += val.as_string();
                }
                else
                {
                    // переменная не найдена — вставляем оригинал ${name}
                    result += "${" + var_name + "}";
                }
            }

            i = end + 1;
        }
        else
        {
            result += text[i];
            i++;
        }
    }

    return result;
}

void write_string(FILE *ptr, const std::string &s)
{
    uint32_t len = static_cast<uint32_t>(s.size());
    fwrite(&len, sizeof(len), 1, ptr);
    if (len > 0)
        fwrite(s.data(), 1, len, ptr);
}

bool read_string(FILE *ptr, std::string &s)
{
    uint32_t len = 0;
    if (fread(&len, sizeof(len), 1, ptr) != 1)
        return false;
    s.resize(len);
    if (len > 0 && fread(&s[0], 1, len, ptr) != len)
        return false;
    return true;
}
void save_vars(FILE *ptr)
{
    std::vector<std::pair<std::string, Var *>> ints, floats, strings, nulls;

    for (auto &pair : variables)
    {
        switch (pair.second.type)
        {
        case Var::Type::Int:
            ints.push_back({pair.first, &pair.second});
            break;
        case Var::Type::Float:
            floats.push_back({pair.first, &pair.second});
            break;
        case Var::Type::String:
            strings.push_back({pair.first, &pair.second});
            break;
        case Var::Type::Null:
            nulls.push_back({pair.first, &pair.second});
            break;
        }
    }

    {
        uint32_t type = static_cast<uint32_t>(Var::Type::Int);
        uint32_t count = static_cast<uint32_t>(ints.size());
        fwrite(&type, sizeof(type), 1, ptr);
        fwrite(&count, sizeof(count), 1, ptr);
        for (auto &kv : ints)
        {
            write_string(ptr, kv.first);
            fwrite(&kv.second->i, sizeof(kv.second->i), 1, ptr);
        }
    }

    {
        uint32_t type = static_cast<uint32_t>(Var::Type::Float);
        uint32_t count = static_cast<uint32_t>(floats.size());
        fwrite(&type, sizeof(type), 1, ptr);
        fwrite(&count, sizeof(count), 1, ptr);
        for (auto &kv : floats)
        {
            write_string(ptr, kv.first);
            fwrite(&kv.second->f, sizeof(kv.second->f), 1, ptr);
        }
    }

    {
        uint32_t type = static_cast<uint32_t>(Var::Type::String);
        uint32_t count = static_cast<uint32_t>(strings.size());
        fwrite(&type, sizeof(type), 1, ptr);
        fwrite(&count, sizeof(count), 1, ptr);
        for (auto &kv : strings)
        {
            write_string(ptr, kv.first);
            write_string(ptr, kv.second->s);
        }
    }

    {
        uint32_t type = static_cast<uint32_t>(Var::Type::Null);
        uint32_t count = static_cast<uint32_t>(nulls.size());
        fwrite(&type, sizeof(type), 1, ptr);
        fwrite(&count, sizeof(count), 1, ptr);
        for (auto &kv : nulls)
        {
            write_string(ptr, kv.first);
        }
    }
}

void load_vars(FILE *ptr)
{
    variables.clear();

    for (int s = 0; s < 4; ++s)
    {
        uint32_t type_raw;
        if (fread(&type_raw, sizeof(type_raw), 1, ptr) != 1)
            return;

        uint32_t count = 0;
        if (fread(&count, sizeof(count), 1, ptr) != 1)
            return;

        Var::Type type = static_cast<Var::Type>(type_raw);

        for (uint32_t i = 0; i < count; ++i)
        {
            std::string key;
            if (!read_string(ptr, key))
                return;

            Var var;
            var.type = type;

            switch (type)
            {
            case Var::Type::Int:
                if (fread(&var.i, sizeof(var.i), 1, ptr) != 1)
                    return;
                break;
            case Var::Type::Float:
                if (fread(&var.f, sizeof(var.f), 1, ptr) != 1)
                    return;
                break;
            case Var::Type::String:
                if (!read_string(ptr, var.s))
                    return;
                break;
            case Var::Type::Null:
                break;
            }

            variables[key] = var;
        }
    }
}