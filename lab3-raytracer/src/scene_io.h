#pragma once

#include <filesystem>

#include "scene.h"

Scene ReadScene(const std::filesystem::path& path);
