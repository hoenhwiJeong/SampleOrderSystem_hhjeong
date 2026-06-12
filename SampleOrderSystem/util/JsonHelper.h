#pragma once
#include "../nlohmann/json.hpp"
#include <string>

class JsonHelper {
public:
    static nlohmann::json Load(const std::string& path);
    static void Save(const std::string& path, const nlohmann::json& j);
};
