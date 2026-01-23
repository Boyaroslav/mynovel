cmake_minimum_required(VERSION 3.16)     # Проверка версии CMake.
                                                                                # Если версия установленой программы
                                                                                # старее указаной, произайдёт аварийный выход.

project(lcnovel)                        # Название проекта
set(CMAKE_CXX_STANDARD 20)

link_directories(sdl SHARED)


add_executable(lcnovel main.cpp)        # Создает исполняемый файл с именем main


target_link_libraries(lcnovel SDL2 SDL2_image SDL2_ttf SDL2_mixer)