#include "utils.hpp"
#include "event_types.hpp"
#include <variant>

using Var = std::variant<int, double, std::string>;

std::unordered_map<std::string, std::string> string_variables;
std::unordered_map<std::string, uint32_t> int_variables;
std::unordered_map<std::string, double> float_variables;

template<typename T>
T get_value(const std::string& t) {
    if constexpr (std::is_same_v<T, int>) {
        auto it = int_variables.find(t);
        if (it != int_variables.end()) return it->second;
        return "default";
    }
    if constexpr (std::is_same_v<T, std::string>) {
        auto it = string_variables.find(t);
        if (it != string_variables.end()) return it->second;
        return "default";
    }
    if constexpr (std::is_same_v<T, float>) {
        auto it = float_variables.find(t);
        if (it != float_variables.end()) return it->second;
        return "default";
    }
}

float get_value(std::string t) {
    return float_variables[t];
}

void set_value(std::string t, int v) {
    int_variables[t] = v;
}


void set_value(std::string t, uint32_t v) {
    int_variables[t] = v;
}

void set_value(std::string t, double v) {
    float_variables[t] = v;
}

void set_value(std::string t, std::string v) {
    string_variables[t] = v;
}

void vars_init() {
    set_value("VAR_BG_CHANGE_SPEED", 1.0);
    set_value("NUIL", 1111);
    set_value("WINDOW_TITLE", "cock novel");
    auto v1 = get_value("PENIS");
    auto v2 = get_value("WINDOW_TITLE");
    std::cout<<"HUIII "<<v1<<" "<<v2<<"\n";
}