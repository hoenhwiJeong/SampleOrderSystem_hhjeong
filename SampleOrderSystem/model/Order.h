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

    // PRODUCING 상태일 때만 유효한 생산 정보 (영속화용)
    int    prodShortage  = 0;
    int    prodActual    = 0;
    double prodTotalTime = 0.0;
    double prodYieldRate = 0.0;

    static Order           fromJson(const nlohmann::json& j);
    nlohmann::json         toJson() const;
    static std::string     statusToString(OrderStatus s);
    static OrderStatus     statusFromString(const std::string& s);
};
