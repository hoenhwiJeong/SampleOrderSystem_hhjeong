#include <gtest/gtest.h>
#include "util/BusinessLogic.h"

// ── calcActualProduction ──────────────────────────────────────────────────

// CLAUDE.md 명세 예시: SiC 파워기판 (수율 0.92, 부족 50ea)
// ceil(50 / (0.92 * 0.9)) = ceil(60.386...) = 61
TEST(CalcActualProduction, ClaudeSpecExample) {
    EXPECT_EQ(BusinessLogic::calcActualProduction(50, 0.92), 61);
}

// 부족분이 정확히 나누어 떨어지는 경우
TEST(CalcActualProduction, ExactDivision) {
    // ceil(81 / (0.9 * 0.9)) = ceil(100.0) = 100
    EXPECT_EQ(BusinessLogic::calcActualProduction(81, 0.9), 100);
}

// 부족분 1 ea (최소값)
TEST(CalcActualProduction, MinimumShortage) {
    EXPECT_GE(BusinessLogic::calcActualProduction(1, 0.92), 1);
}

// ── calcTotalTime ─────────────────────────────────────────────────────────

// CLAUDE.md 명세 예시: 0.8 min/ea × 61 = 48.8 min
TEST(CalcTotalTime, ClaudeSpecExample) {
    EXPECT_DOUBLE_EQ(BusinessLogic::calcTotalTime(0.8, 61), 48.8);
}

TEST(CalcTotalTime, ZeroProduction) {
    EXPECT_DOUBLE_EQ(BusinessLogic::calcTotalTime(1.0, 0), 0.0);
}

// ── calcStockAfterProduction ──────────────────────────────────────────────

// CLAUDE.md 명세 예시: 30 + 61 - 80 = 11 ea
TEST(CalcStockAfterProduction, ClaudeSpecExample) {
    EXPECT_EQ(BusinessLogic::calcStockAfterProduction(30, 61, 80), 11);
}

// 생산량이 주문량보다 많은 경우 (재고 증가)
TEST(CalcStockAfterProduction, SurplusProduction) {
    EXPECT_EQ(BusinessLogic::calcStockAfterProduction(0, 100, 80), 20);
}

// ── calcReservedStock ─────────────────────────────────────────────────────

// PRODUCING 주문 없을 때 예약분 = 0
TEST(CalcReservedStock, NoProducingOrders) {
    std::vector<Order> orders;
    EXPECT_EQ(BusinessLogic::calcReservedStock(orders, "S-001"), 0);
}

// 다른 시료의 PRODUCING 주문은 예약분에 포함 안 됨
TEST(CalcReservedStock, OtherSampleIgnored) {
    Order o;
    o.sampleId     = "S-002";
    o.quantity     = 100;
    o.prodShortage = 50;
    o.status       = OrderStatus::PRODUCING;

    EXPECT_EQ(BusinessLogic::calcReservedStock({o}, "S-001"), 0);
}

// Bug2 시나리오: A(100ea, 부족50) → 예약분 = 100-50 = 50
TEST(CalcReservedStock, Bug2ScenarioOrderA) {
    Order a;
    a.sampleId     = "S-001";
    a.quantity     = 100;
    a.prodShortage = 50;
    a.status       = OrderStatus::PRODUCING;

    EXPECT_EQ(BusinessLogic::calcReservedStock({a}, "S-001"), 50);
}

// ── calcAvailableStock ────────────────────────────────────────────────────

// Bug2 시나리오 전체: A 예약 후 B 승인 가용재고 = 0
TEST(CalcAvailableStock, Bug2ScenarioFullFlow) {
    Order a;
    a.sampleId     = "S-001";
    a.quantity     = 100;
    a.prodShortage = 50;
    a.status       = OrderStatus::PRODUCING;

    int reserved  = BusinessLogic::calcReservedStock({a}, "S-001");
    int available = BusinessLogic::calcAvailableStock(50, reserved);

    EXPECT_EQ(reserved,  50);
    EXPECT_EQ(available,  0);
}

// 예약분이 재고보다 커도 가용재고는 0 이하로 내려가지 않음
TEST(CalcAvailableStock, NeverNegative) {
    EXPECT_EQ(BusinessLogic::calcAvailableStock(10, 100), 0);
}

// 예약 없는 경우: 가용재고 = 현재재고
TEST(CalcAvailableStock, NoReservation) {
    EXPECT_EQ(BusinessLogic::calcAvailableStock(200, 0), 200);
}
