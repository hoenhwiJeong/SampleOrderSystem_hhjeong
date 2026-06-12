#pragma once
#include "../model/Order.h"
#include "../model/StockInfo.h"
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

namespace BusinessLogic {

    // 공정 오차 10% 안전 마진 (불량률 보정 계수)
    constexpr double DEFECT_RATE_MARGIN = 0.9;

    // 실 생산량: ceil(부족분 / (수율 × DEFECT_RATE_MARGIN))
    int calcActualProduction(int shortage, double yieldRate);

    // 총 생산시간: 평균생산시간(min/ea) × 실 생산량
    double calcTotalTime(double avgProductionTime, int actualProduction);

    // PRODUCING 주문들이 선점한 재고 합계
    // 예약분 = quantity - prodShortage (승인 당시 있던 재고)
    int calcReservedStock(const std::vector<Order>& orders,
                          const std::string& sampleId);

    // 가용 재고: max(0, 현재 재고 - 예약분)
    int calcAvailableStock(int currentStock, int reservedStock);

    // 생산 완료 후 재고: 현재재고 + 실생산량 - 주문수량
    int calcStockAfterProduction(int currentStock,
                                 int actualProduction,
                                 int orderQuantity);

    // 시료별 재고 상태 목록 (고갈/부족/여유 판단 포함)
    std::vector<StockInfo> buildStockInfoList(const std::vector<Sample>& samples,
                                              const std::vector<Order>&  orders);

} // namespace BusinessLogic
