#pragma once
#include "../model/Order.h"
#include "../model/Sample.h"
#include <vector>
#include <string>

struct StockInfo {
    Sample      sample;
    std::string status;     // "여유" / "부족" / "고갈"
    int         confirmedTotal;
};

class MonitorView {
public:
    int  showSubMenu();
    void showOrderStats(const std::vector<Order>& orders);
    void showStockStats(const std::vector<StockInfo>& stocks);
    void showProducingQueue(const std::vector<Order>& producing,
                            const std::vector<Sample>& samples);
};
