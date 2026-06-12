#pragma once
#include <gmock/gmock.h>
#include "view/MonitorView.h"

class MockMonitorView : public MonitorView {
public:
    MOCK_METHOD(int,  showSubMenu,    (),                                                       (override));
    MOCK_METHOD(void, showOrderStats, (const std::vector<Order>&, const std::vector<StockInfo>&), (override));
    MOCK_METHOD(void, showStockStats, (const std::vector<StockInfo>&),                           (override));
};
