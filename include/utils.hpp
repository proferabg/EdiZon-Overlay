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

/* Returns the hex string representation of a provided number */
template<typename T>
std::string toHexString(T value, size_t width = sizeof(T) * 2) {
    std::stringstream sstream;
    sstream << std::uppercase << std::setfill('0') << std::setw(width) << std::hex << value;
    return sstream.str();
}

bool isServiceRunning(const char *serviceName) {
  Handle handle;
  SmServiceName service_name = smEncodeName(serviceName);
  bool running = R_FAILED(smRegisterService(&handle, service_name, false, 1));

  svcCloseHandle(handle);

  if (!running)
    smUnregisterService(service_name);

  return running;
}