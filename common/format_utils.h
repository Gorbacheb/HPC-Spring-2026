#pragma once

#include <iomanip>
#include <sstream>
#include <string>

inline std::string ToString(double value, int precision = 6) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}
