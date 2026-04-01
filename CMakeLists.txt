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
    FetchContent_Declare(freetype
    URL https://download.savannah.gnu.org/releases/freetype/freetype-2.13.2.tar.gz
    )
    FetchContent_Declare(SDL2
        URL https://github.com/libsdl-org/SDL/releases/download/release-2.32.4/SDL2-2.32.4.tar.gz
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
    set(SDL2TTF_FREETYPE ON CACHE BOOL "" FORCE)
    set(SDL2TTF_HARFBUZZ OFF CACHE BOOL "" FORCE)
    set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
    set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
    set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
    set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
    set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
    set(SDL2MIXER_MOD OFF CACHE BOOL "" FORCE)
    set(SDL2MIXER_MOD_XMP OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(SDL2MIXER_FLAC ON CACHE BOOL "" FORCE)
    set(SDL2MIXER_MOD OFF CACHE BOOL "" FORCE)
    set(SDL2MIXER_MOD_XMP OFF CACHE BOOL "" FORCE)
    set(SDL2MIXER_MP3 ON CACHE BOOL "" FORCE)
    set(SDL2MIXER_OGG ON CACHE BOOL "" FORCE)
    set(SDL2MIXER_OPUS OFF CACHE BOOL "" FORCE) # господи хоть бы денис опять хуеты на опусе не зарендерил
    set(SDL2MIXER_WAVPACK OFF CACHE BOOL "" FORCE)
    set(SDL2MIXER_FLAC_LIBFLAC OFF CACHE BOOL "" FORCE)
    set(SDL2MIXER_FLAC_DRFLAC ON CACHE BOOL "" FORCE)
    set(SDL2MIXER_MP3_MPG123 OFF CACHE BOOL "" FORCE)
    set(SDL2MIXER_MP3_DRMP3 ON CACHE BOOL "" FORCE)
    set(SDL2MIXER_OGG_TREMOR OFF CACHE BOOL "" FORCE)
    set(SDL2MIXER_VORBIS_STB ON CACHE BOOL "" FORCE)
    set(SDL2MIXER_OPUS_SHARED OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(freetype)

    set(FREETYPE_LIBRARY freetype)
    set(FREETYPE_INCLUDE_DIRS
        ${freetype_SOURCE_DIR}/include
        ${freetype_BINARY_DIR}/include
    )

    if(NOT TARGET Freetype::Freetype)
        add_library(Freetype::Freetype ALIAS freetype)
    endif()
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
if(WIN32)
    target_link_options(lcnovel PRIVATE -mconsole)
    target_link_options(dumper PRIVATE -mconsole)
endif()

target_link_libraries(lcnovel ${SDL_LIBS} lua_lib)
target_link_libraries(dumper ${SDL_LIBS})