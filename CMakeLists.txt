cmake_minimum_required(VERSION 3.16)

project(lcnovel)
set(CMAKE_CXX_STANDARD 20)
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")


find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)

add_executable(lcnovel main.cpp)

add_executable(dumper novel_dumper.cpp)

target_link_libraries(lcnovel
    SDL2::SDL2
    SDL2_image::SDL2_image
    SDL2_ttf::SDL2_ttf
    SDL2_mixer::SDL2_mixer
)


target_link_libraries(dumper
    SDL2::SDL2
    SDL2_image::SDL2_image
    SDL2_ttf::SDL2_ttf
    SDL2_mixer::SDL2_mixer
)
