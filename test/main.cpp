#include "runtime.hpp"

static runtime core {};

int32_t main(int, char**) {
    core.init();
    core.mainloop();
    core.quit();

    return 0;
}