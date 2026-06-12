#pragma once
#include "Sample.h"
#include <string>

struct StockInfo {
    Sample      sample;
    std::string status;         // "여유" / "부족" / "고갈"
    int         confirmedTotal;
    int         reservedStock = 0; // PRODUCING 주문이 선점한 재고 합계
};
