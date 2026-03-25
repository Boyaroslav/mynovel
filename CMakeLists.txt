cmake_minimum_required(VERSION 3.16)
project(lcnovel)
set(CMAKE_CXX_STANDARD 20)

option(SYSTEM_SDL "Use system-installed SDL2 instead of building from source" OFF)

if(SYSTEM_SDL)
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")
    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)
    find_package(SDL2_ttf REQUIRED)
    find_package(SDL2_mixer REQUIRED)
    set(SDL_LIBS SDL2::SDL2 SDL2_image::SDL2_image SDL2_ttf::SDL2_ttf SDL2_mixer::SDL2_mixer)
else()
    include(FetchContent)
    FetchContent_Declare(SDL2
        URL https://github.com/libsdl-org/SDL/releases/download/release-2.30.0/SDL2-2.30.0.tar.gz
    )
    FetchContent_Declare(SDL2_image
        URL https://github.com/libsdl-org/SDL_image/releases/download/release-2.8.2/SDL2_image-2.8.2.tar.gz
    )
    FetchContent_Declare(SDL2_ttf
        URL https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.22.0/SDL2_ttf-2.22.0.tar.gz
    )
    FetchContent_Declare(SDL2_mixer
        URL https://github.com/libsdl-org/SDL_mixer/releases/download/release-2.8.0/SDL2_mixer-2.8.0.tar.gz
    )
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_PIPEWIRE OFF CACHE BOOL "" FORCE)
    set(SDL2IMAGE_STATIC ON CACHE BOOL "" FORCE)
    set(SDL2IMAGE_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL2TTF_STATIC ON CACHE BOOL "" FORCE)
    set(SDL2TTF_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL2MIXER_STATIC ON CACHE BOOL "" FORCE)
    set(SDL2MIXER_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL2MIXER_OPUS OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(SDL2 SDL2_image SDL2_ttf SDL2_mixer)
    set(SDL_LIBS SDL2::SDL2-static SDL2::image SDL2::ttf SDL2::mixer)
endif()

file(GLOB LUA_SOURCES lua/*.c)
list(REMOVE_ITEM LUA_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/lua/lua.c)
list(REMOVE_ITEM LUA_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/lua/luac.c)
list(REMOVE_ITEM LUA_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/lua/onelua.c)
add_library(lua_lib STATIC ${LUA_SOURCES})
target_include_directories(lua_lib PUBLIC lua/)

add_executable(lcnovel main.cpp)
add_executable(dumper novel_dumper.cpp)

target_link_libraries(lcnovel ${SDL_LIBS} lua_lib)
target_link_libraries(dumper ${SDL_LIBS})