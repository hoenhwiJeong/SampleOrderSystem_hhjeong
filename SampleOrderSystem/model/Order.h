#pragma once
#include "../nlohmann/json.hpp"
#include <string>

enum class OrderStatus {
    RESERVED,
    PRODUCING,
    CONFIRMED,
    RELEASED,
    REJECTED
};

struct Order {
    std::string id;           // ORD-YYYYMMDD-NNNN
    std::string sampleId;
    std::string customerName;
    int         quantity;
    OrderStatus status;

    static Order           fromJson(const nlohmann::json& j);
    nlohmann::json         toJson() const;
    static std::string     statusToString(OrderStatus s);
    static OrderStatus     statusFromString(const std::string& s);
};
