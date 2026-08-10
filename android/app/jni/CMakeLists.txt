cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project("lcnovel")

set(PROJECT_ROOT ${CMAKE_SOURCE_DIR}/../../..)
set(LUA_ROOT ${CMAKE_SOURCE_DIR}/../../../lua)
set(JNI_LIBS_OUT_DIR ${CMAKE_SOURCE_DIR}/../src/main/jniLibs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${JNI_LIBS_OUT_DIR})

include(FetchContent)

set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL2IMAGE_STATIC OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_SHARED ON CACHE BOOL "" FORCE)
set(SDL2TTF_STATIC OFF CACHE BOOL "" FORCE)
set(SDL2TTF_SHARED ON CACHE BOOL "" FORCE)
set(SDL2MIXER_STATIC OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_SHARED ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
set(SDL2TTF_FREETYPE ON CACHE BOOL "" FORCE)

set(SDL2MIXER_STATIC OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_SHARED ON CACHE BOOL "" FORCE)
set(SDL2MIXER_MP3 ON CACHE BOOL "" FORCE)
set(SDL2MIXER_OGG ON CACHE BOOL "" FORCE)
set(SDL2MIXER_OPUS OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_MOD OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_MOD_XMP OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_FLAC OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_WAVPACK OFF CACHE BOOL "" FORCE)

FetchContent_Declare(SDL2       URL https://github.com/libsdl-org/SDL/releases/download/release-2.32.4/SDL2-2.32.4.tar.gz)
FetchContent_Declare(SDL2_image URL https://github.com/libsdl-org/SDL_image/releases/download/release-2.8.2/SDL2_image-2.8.2.tar.gz)
FetchContent_Declare(SDL2_ttf   URL https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.22.0/SDL2_ttf-2.22.0.tar.gz)
FetchContent_Declare(SDL2_mixer URL https://github.com/libsdl-org/SDL_mixer/releases/download/release-2.8.0/SDL2_mixer-2.8.0.tar.gz)
FetchContent_MakeAvailable(SDL2 SDL2_image SDL2_ttf SDL2_mixer)

file(GLOB LUA_SOURCES ${LUA_ROOT}/*.c)
list(REMOVE_ITEM LUA_SOURCES ${LUA_ROOT}/lua.c ${LUA_ROOT}/luac.c ${LUA_ROOT}/onelua.c)
add_library(lua SHARED ${LUA_SOURCES})
target_include_directories(lua PUBLIC ${LUA_ROOT})
target_compile_definitions(lua PRIVATE LUA_USE_LINUX LUA_COMPAT_5_2 LUA_COMPAT_5_3 LUA_USE_DLOPEN=0)
target_compile_options(lua PRIVATE -w -fno-stack-protector)

add_library(lcnovel SHARED
    ${CMAKE_SOURCE_DIR}/../../../main.cpp
)

target_link_libraries(lcnovel
    SDL2
    SDL2_image
    SDL2_ttf
    SDL2_mixer
    lua
    android
    OpenSLES
    log
    z
    m
)

target_include_directories(lcnovel PRIVATE
    ${sdl2_SOURCE_DIR}/include
    ${sdl2_image_SOURCE_DIR}/include
    ${sdl2_ttf_SOURCE_DIR}
    ${sdl2_mixer_SOURCE_DIR}/include
    ${LUA_ROOT}
    src/main/cpp
    ${PROJECT_ROOT}
)

target_compile_options(lcnovel PRIVATE -Wall -Wextra -fPIC)

add_custom_command(TARGET lcnovel POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${JNI_LIBS_OUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:lcnovel>
        $<TARGET_FILE:SDL2>
        $<TARGET_FILE:SDL2_image>
        $<TARGET_FILE:SDL2_ttf>
        $<TARGET_FILE:SDL2_mixer>
        $<TARGET_FILE:lua>
        ${JNI_LIBS_OUT_DIR}
    COMMENT "Copying built shared libraries to jniLibs/${ANDROID_ABI}"
)