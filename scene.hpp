#include "utils.hpp"
#include "event.hpp"

class Scene{
    public:
    str name;
    str title;

    template<typename... Args>
    void add_event(uint8_t command, Args... args) {

        // Печать всех аргументов (если есть)
        ((std::cout << "- " << args << "\n"), ...);
    }


    Scene(str n){ name = n;}

};