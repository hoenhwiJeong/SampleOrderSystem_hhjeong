#include "Sample.h"

Sample Sample::fromJson(const nlohmann::json& j) {
    Sample s;
    s.id                = j.at("id").get<std::string>();
    s.name              = j.at("name").get<std::string>();
    s.avgProductionTime = j.at("avgProductionTime").get<double>();
    s.yieldRate         = j.at("yieldRate").get<double>();
    s.stock             = j.at("stock").get<int>();
    return s;
}

nlohmann::json Sample::toJson() const {
    return {
        {"id",                id},
        {"name",              name},
        {"avgProductionTime", avgProductionTime},
        {"yieldRate",         yieldRate},
        {"stock",             stock}
    };
}
