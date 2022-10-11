#include "runtime.hpp"

bool timing::reach(uint64_t ms) {
    this->delta_ticks = SDL_GetTicks64() - this->elapsed_ticks;
    return this->delta_ticks > ms;
}

bool timing::reset() {
    this->elapsed_ticks = SDL_GetTicks64();
    return false;
}
