#ifndef EKG_UTIL_ENV_H
#define EKG_UTIL_ENV_H

#include <iostream>
#include <vector>

namespace ekg {
    void log(const std::string &log);
    bool read_file(std::string_view path, std::string &file_string_data);
    bool read_file(std::string_view path, std::vector<char> &buffer);
}

#endif