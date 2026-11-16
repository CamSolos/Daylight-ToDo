#pragma once

#include "Task.h"
#include <filesystem>
#include <vector>

class FileHandler {
public:
    static void Save(const std::vector<Task>& tasks, const std::filesystem::path& filePath);
    static std::vector<Task> Load(const std::filesystem::path& filePath);
};
