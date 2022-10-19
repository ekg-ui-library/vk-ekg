#include "ekg/util/env.hpp"
#include <fstream>

void ekg::log(const std::string &log) {
    const std::string full_log = "[ekg] " + log;
    std::cout << full_log.c_str() << '\n';
}

bool ekg::read_file(std::string_view path, std::string &file_string_data) {
    std::ifstream ifs {path.data()};

    if (ifs.is_open()) {
        std::string string_buffer {};
        while (std::getline(ifs, string_buffer)) {
            file_string_data += string_buffer;
            file_string_data += '\n';
        }

        ifs.close();
        return true;
    }

    return false;
}

bool ekg::read_file(std::string_view path, std::vector<char> &buffer) {
    std::ifstream ifs {path.data()};
    if (ifs.is_open()) {
        size_t file_size {(size_t) ifs.tellg()};
        buffer.resize(file_size);

        ifs.seekg(0);
        ifs.read(buffer.data(), (int64_t) file_size);
        ifs.close();
        return true;
    }

    return false;
}
