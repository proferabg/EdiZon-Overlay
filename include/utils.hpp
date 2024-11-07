#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <cstring>

/* C++ style sprintf */
template <typename ...Args>
std::string formatString(const std::string& format, Args && ...args) {
    auto size = std::snprintf(nullptr, 0, format.c_str(), args...);
    char output[size + 1];
    std::memset(output, ' ', sizeof(output));
    std::sprintf(output, format.c_str(), args...);

    return output;
}