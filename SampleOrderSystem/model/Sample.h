#pragma once
#include "../nlohmann/json.hpp"
#include <string>

struct Sample {
    std::string id;
    std::string name;
    double      avgProductionTime;  // min/ea
    double      yieldRate;          // 0.0 ~ 1.0
    int         stock;              // ea

    static Sample        fromJson(const nlohmann::json& j);
    nlohmann::json       toJson() const;
};
