#include <SDL2/SDL.h>
#include "utils.hpp"
#include "event.hpp"



class Screen{
    private:
    SDL_Surface* surface;
    SDL_Window* window;
    SDL_Renderer* renderer;

    vecev events;
    
    public:
    SDL_Event e;
    Screen(){}
    void init_(){
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        log("sdl init error");
        return;
        }

        window = SDL_CreateWindow("lcnovel",
                                         SDL_WINDOWPOS_CENTERED,
                                         SDL_WINDOWPOS_CENTERED,
                                         width, height,
                                         SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        surface = SDL_GetWindowSurface(window);
    }

    void clean(){
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void load_(str name){
        return;
    }

    void run(abool& r){
        while (r){
            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) {
                    r = false;
                }
            SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));
            SDL_UpdateWindowSurface(window);
            }

        }
        return;
    }

    
};