#pragma once
#include "../model/Order.h"
#include "../model/StockInfo.h"
#include <vector>
#include <string>

class MonitorView {
public:
    virtual ~MonitorView() = default;

    virtual int  showSubMenu();
    virtual void showOrderStats(const std::vector<Order>& orders,
                                const std::vector<StockInfo>& stocks);
    virtual void showStockStats(const std::vector<StockInfo>& stocks);
};
