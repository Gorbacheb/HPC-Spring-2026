#pragma once

#include <csv2/writer.hpp>

#include <fstream>
#include <string>
#include <vector>

inline bool WriteCsv(const std::string &path, const std::vector<std::vector<std::string>> &rows) {
    std::ofstream stream(path);
    if (!stream) {
        return false;
    }
    csv2::Writer<csv2::delimiter<','>> writer(stream);
    writer.write_rows(rows);
    return static_cast<bool>(stream);
}
