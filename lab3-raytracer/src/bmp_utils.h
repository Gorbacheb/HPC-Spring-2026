#pragma once

#include <string>
#include <vector>

#include "vector.h"

void SaveBmp(const std::string& path, const std::vector<Vector>& pixels, int width, int height);
