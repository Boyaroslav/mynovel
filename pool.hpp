#include "utils.hpp"

str pool;

int add_to_pool(str s){
    int r = pool.size();
    pool += s; pool += '\0';
    return r;
}