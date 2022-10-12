#ifndef EKG_APP_H
#define EKG_APP_H

#include <SDL2/SDL.h>
#include "ekg/gpu/gpu_vk_renderer.hpp"

class runtime {
protected:
    SDL_DisplayMode sdl_display_mode {};
    SDL_Window* sdl_window {};

    bool mainloop_running {false};
    ekg::gpu::vk_renderer renderer {};
public:
    SDL_DisplayMode &get_display_mode();
    SDL_Window* get_sdl_window();

    void init();
    void mainloop();
    void quit();
};

#endif