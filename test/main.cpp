#include "ekg/gpu/gpu_vk.hpp"
#include "runtime.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

runtime* const core {new runtime()};

int32_t main(int, char**) {
    SDL_Init(SDL_INIT_VIDEO);

    core->sdl_mode.w = 1280;
    core->sdl_mode.h = 800;
    core->sdl_window = SDL_CreateWindow("vk ekg", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, core->sdl_mode.w, core->sdl_mode.h, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);

    timing timing_framerate {};
    timing timing_fps_count {};

    uint64_t fps = 1000 / 60;
    uint32_t elapsed_frames {};
    uint32_t display_fps {};

    SDL_Event sdl_event {};
    core->mainloop = true;

    while (core->mainloop) {
        if (timing_framerate.reach(fps) && timing_framerate.reset()) {
            if (timing_fps_count.reach(1000) && timing_fps_count.reset()) {
                display_fps = elapsed_frames;
                elapsed_frames = 0;
            }

            core->dt = static_cast<float>(timing_framerate.delta_ticks) / 100;
            while (SDL_PollEvent(&sdl_event)) {
                switch (sdl_event.type) {
                    case SDL_QUIT: {
                        core->mainloop = false;
                        break;
                    }
                }
            }

            elapsed_frames++;
        }
    }

    return 0;
}