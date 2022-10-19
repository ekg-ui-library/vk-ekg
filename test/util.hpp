#ifndef EKG_UTIL_ENV_H
#define EKG_UTIL_ENV_H

#include <iostream>

namespace util {
    extern float dt;

    struct timing {
        uint64_t elapsed_ticks {};
        uint64_t delta_ticks {};

        bool reach(uint64_t ms);
        bool reset();
    };

    void log(std::string_view log);
}

#endif