#include "runtime.hpp"
#include "util.hpp"

SDL_DisplayMode &runtime::get_display_mode() {
    return this->sdl_display_mode;
}

SDL_Window *runtime::get_sdl_window() {
    return this->sdl_window;
}

void runtime::init() {
    util::log("initialising vk gpu test");
    SDL_Init(SDL_INIT_VIDEO);

    this->sdl_window = SDL_CreateWindow("vk gpu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (this->sdl_display_mode.w = 1280), (this->sdl_display_mode.h = 800), SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
    if (this->renderer.init(this->sdl_window)) util::log("could not create vk instance");
    this->renderer.debug_message();
    this->renderer.create_surface(this->sdl_window);
    this->mainloop_running = true;
}

void runtime::mainloop() {
    util::timing timing_framerate {};
    util::timing timing_fps_count {};

    uint64_t fps = 1000 / 60;
    uint32_t elapsed_frames {};
    uint32_t display_fps {};
    SDL_Event sdl_event {};

    while (this->mainloop_running) {
        if (timing_framerate.reach(fps) && timing_framerate.reset()) {
            if (timing_fps_count.reach(1000) && timing_fps_count.reset()) {
                display_fps = elapsed_frames;
                elapsed_frames = 0;
            }

            util::dt = static_cast<float>(timing_framerate.delta_ticks) / 100;
            while (SDL_PollEvent(&sdl_event)) {
                switch (sdl_event.type) {
                    case SDL_QUIT: {
                        this->mainloop_running = false;
                        break;
                    }
                }
            }

            elapsed_frames++;
        }
    }
}

void runtime::quit() {
    this->renderer.quit();
}
