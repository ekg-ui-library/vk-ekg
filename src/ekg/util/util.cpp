#include "ekg/util/util.hpp"

void ekg::log(const std::string &log) {
    const std::string full_log = "[ekg] " + log;
    std::cout << full_log.c_str() << '\n';
}