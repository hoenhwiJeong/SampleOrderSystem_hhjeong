#include "OrderRepository.h"
#include "../util/JsonHelper.h"
#include <stdexcept>

OrderRepository::OrderRepository(const std::string& filePath)
    : filePath_(filePath) {
    load();
}

void OrderRepository::load() {
    auto j = JsonHelper::Load(filePath_);
    data_.clear();
    for (const auto& item : j)
        data_.push_back(Order::fromJson(item));
}

void OrderRepository::save() {
    nlohmann::json j = nlohmann::json::array();
    for (const auto& o : data_)
        j.push_back(o.toJson());
    JsonHelper::Save(filePath_, j);
}

void OrderRepository::add(const Order& o) {
    for (const auto& existing : data_)
        if (existing.id == o.id)
            throw std::runtime_error("이미 존재하는 주문 ID입니다: " + o.id);
    data_.push_back(o);
    save();
}

std::vector<Order> OrderRepository::findAll() const {
    return data_;
}

std::optional<Order> OrderRepository::findById(const std::string& id) const {
    for (const auto& o : data_)
        if (o.id == id) return o;
    return std::nullopt;
}

std::vector<Order> OrderRepository::findByStatus(OrderStatus status) const {
    std::vector<Order> result;
    for (const auto& o : data_)
        if (o.status == status) result.push_back(o);
    return result;
}

void OrderRepository::update(const Order& o) {
    for (auto& existing : data_) {
        if (existing.id == o.id) {
            existing = o;
            save();
            return;
        }
    }
    throw std::runtime_error("주문을 찾을 수 없습니다: " + o.id);
}

int OrderRepository::countByDate(const std::string& yyyymmdd) const {
    std::string prefix = "ORD-" + yyyymmdd + "-";
    int count = 0;
    for (const auto& o : data_)
        if (o.id.rfind(prefix, 0) == 0) ++count;
    return count;
}
