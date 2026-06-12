#include "JsonHelper.h"
#include <fstream>
#include <stdexcept>

nlohmann::json JsonHelper::Load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open())
        return nlohmann::json::array();

    nlohmann::json j;
    try {
        file >> j;
    } catch (const nlohmann::json::parse_error&) {
        return nlohmann::json::array();
    }
    return j;
}

void JsonHelper::Save(const std::string& path, const nlohmann::json& j) {
    std::ofstream file(path);
    if (!file.is_open())
        throw std::runtime_error("파일을 열 수 없습니다: " + path);
    file << j.dump(4);
}
