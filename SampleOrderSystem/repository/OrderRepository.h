#pragma once
#include "../model/Order.h"
#include <vector>
#include <optional>
#include <string>

class OrderRepository {
public:
    explicit OrderRepository(const std::string& filePath);

    void                     add(const Order& o);
    std::vector<Order>       findAll() const;
    std::optional<Order>     findById(const std::string& id) const;
    std::vector<Order>       findByStatus(OrderStatus status) const;
    void                     update(const Order& o);
    int                      countByDate(const std::string& yyyymmdd) const;

private:
    std::string        filePath_;
    std::vector<Order> data_;

    void load();
    void save();
};
