#include "utils.hpp"
#include "event_types.hpp"
#include <variant>

u_int32_t last_id = -1;

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
    r.f = v;
    return r;
}
Var make_var(std::string v)
{
    Var r;
    r.type = Var::Type::String;
    r.s = v;
    return r;
}

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
    set_value("VAR_BG_CHANGE_SPEED", 5.0);
    set_value("WINDOW_TITLE", std::string("my novel"));
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