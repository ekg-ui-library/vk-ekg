#include "util.hpp"
#include <SDL2/SDL.h>

float util::dt {};

void util::log(std::string_view log) {
    const std::string full_log = "[vkgpu] " + std::string(log.data());
    std::cout << full_log.c_str() << '\n';
}

bool util::timing::reach(uint64_t ms) {
    this->delta_ticks = SDL_GetTicks64() - this->elapsed_ticks;
    return this->delta_ticks > ms;
}

bool util::timing::reset() {
    this->elapsed_ticks = SDL_GetTicks64();
    return true;
}