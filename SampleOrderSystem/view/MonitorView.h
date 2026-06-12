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
    // option 1: 주문 현황 + 재고 현황 통합 표시
    void showOrderStats(const std::vector<Order>& orders,
                        const std::vector<StockInfo>& stocks);
    // option 2: 재고 현황만 표시
    void showStockStats(const std::vector<StockInfo>& stocks);
};
