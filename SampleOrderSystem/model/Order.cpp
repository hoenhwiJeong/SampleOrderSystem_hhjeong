#include "Order.h"
#include <stdexcept>

std::string Order::statusToString(OrderStatus s) {
    switch (s) {
        case OrderStatus::RESERVED:  return "RESERVED";
        case OrderStatus::PRODUCING: return "PRODUCING";
        case OrderStatus::CONFIRMED: return "CONFIRMED";
        case OrderStatus::RELEASED:  return "RELEASED";
        case OrderStatus::REJECTED:  return "REJECTED";
        default: throw std::runtime_error("알 수 없는 OrderStatus");
    }
}

OrderStatus Order::statusFromString(const std::string& s) {
    if (s == "RESERVED")  return OrderStatus::RESERVED;
    if (s == "PRODUCING") return OrderStatus::PRODUCING;
    if (s == "CONFIRMED") return OrderStatus::CONFIRMED;
    if (s == "RELEASED")  return OrderStatus::RELEASED;
    if (s == "REJECTED")  return OrderStatus::REJECTED;
    throw std::runtime_error("알 수 없는 상태 문자열: " + s);
}

Order Order::fromJson(const nlohmann::json& j) {
    Order o;
    o.id           = j.at("id").get<std::string>();
    o.sampleId     = j.at("sampleId").get<std::string>();
    o.customerName = j.at("customerName").get<std::string>();
    o.quantity     = j.at("quantity").get<int>();
    o.status       = statusFromString(j.at("status").get<std::string>());
    return o;
}

nlohmann::json Order::toJson() const {
    return {
        {"id",           id},
        {"sampleId",     sampleId},
        {"customerName", customerName},
        {"quantity",     quantity},
        {"status",       statusToString(status)}
    };
}
