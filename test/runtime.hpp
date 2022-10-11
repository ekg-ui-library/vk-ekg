#ifndef EKG_APP_H
#define EKG_APP_H

#include <SDL2/SDL.h>

struct timing {
    uint64_t elapsed_ticks {};
    uint64_t delta_ticks {};

    bool reach(uint64_t ms);
    bool reset();
};

struct runtime {
    SDL_DisplayMode sdl_mode {};
    SDL_Window* sdl_window {};

    float dt {};
    bool mainloop {};
};

extern runtime* const core;

#endif