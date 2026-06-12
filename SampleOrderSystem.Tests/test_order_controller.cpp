#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <filesystem>
#include "controller/OrderController.h"
#include "util/BusinessLogic.h"
#include "mock/MockOrderView.h"
#include "mock/MockMonitorView.h"
#include "mock/MockProductionLineView.h"

using ::testing::Return;
using ::testing::_;
using ::testing::NiceMock;

// ── BusinessLogic::buildStockInfoList 재고 상태 판단 ──────────────────────────

static Sample makeSample(const std::string& id, int stock) {
    Sample s;
    s.id = id; s.name = "시료-" + id;
    s.avgProductionTime = 0.5; s.yieldRate = 0.9; s.stock = stock;
    return s;
}

static Order makeOrder(const std::string& sampleId, int qty, OrderStatus st,
                       int prodShortage = 0) {
    Order o;
    o.id = "ORD-T"; o.sampleId = sampleId;
    o.customerName = "고객"; o.quantity = qty; o.status = st;
    o.prodShortage = prodShortage;
    return o;
}

TEST(BuildStockInfoList, StockZero_Exhausted) {
    Sample s = makeSample("S-001", 0);
    auto result = BusinessLogic::buildStockInfoList({s}, {});
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].status, "고갈");
}

TEST(BuildStockInfoList, StockLessThanConfirmed_Insufficient) {
    Sample s = makeSample("S-001", 5);
    Order confirmed = makeOrder("S-001", 10, OrderStatus::CONFIRMED);
    auto result = BusinessLogic::buildStockInfoList({s}, {confirmed});
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].status, "부족");
    EXPECT_EQ(result[0].confirmedTotal, 10);
}

TEST(BuildStockInfoList, StockSufficient_Available) {
    Sample s = makeSample("S-001", 20);
    Order confirmed = makeOrder("S-001", 10, OrderStatus::CONFIRMED);
    auto result = BusinessLogic::buildStockInfoList({s}, {confirmed});
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].status, "여유");
}

TEST(BuildStockInfoList, ProducingOrder_ReservedStockCalculated) {
    Sample s = makeSample("S-001", 50);
    // PRODUCING: quantity=100, prodShortage=50 → 예약분 = 100-50 = 50
    Order producing = makeOrder("S-001", 100, OrderStatus::PRODUCING, 50);
    auto result = BusinessLogic::buildStockInfoList({s}, {producing});
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].reservedStock, 50);
}

// ── OrderController 승인·출고 분기 ────────────────────────────────────────────

class OrderControllerTest : public ::testing::Test {
protected:
    const std::string sPath = "test_tmp_oc_s.json";
    const std::string oPath = "test_tmp_oc_o.json";

    void SetUp() override {
        std::filesystem::remove(sPath);
        std::filesystem::remove(oPath);
    }
    void TearDown() override {
        std::filesystem::remove(sPath);
        std::filesystem::remove(oPath);
    }

    void addSample(const std::string& id, int stock,
                   double yield = 0.9, double prodTime = 0.5) {
        SampleRepository repo(sPath);
        Sample s;
        s.id = id; s.name = "시료-" + id;
        s.avgProductionTime = prodTime; s.yieldRate = yield; s.stock = stock;
        repo.add(s);
    }

    void addOrder(const std::string& id, const std::string& sampleId,
                  int qty, OrderStatus status = OrderStatus::RESERVED) {
        OrderRepository repo(oPath);
        Order o;
        o.id = id; o.sampleId = sampleId; o.customerName = "고객";
        o.quantity = qty; o.status = status;
        repo.add(o);
    }
};

// 재고 충분 → CONFIRMED, stock 차감
TEST_F(OrderControllerTest, Approval_SufficientStock_BecomesConfirmed) {
    addSample("S-001", 200);
    addOrder("ORD-T001", "S-001", 100);

    SampleRepository           sRepo(sPath);
    OrderRepository            oRepo(oPath);
    ProductionLineService      svc;
    NiceMock<MockOrderView>    mockOrderView;
    NiceMock<MockMonitorView>  mockMonitor;
    NiceMock<MockProductionLineView> mockProd;

    OrderController ctrl(sRepo, oRepo, svc, mockOrderView, mockMonitor, mockProd);

    EXPECT_CALL(mockOrderView, showReservedList(_, _)).WillOnce(Return(1));
    EXPECT_CALL(mockOrderView, showApprovalDetail(_, _, _, _, _)).WillOnce(Return('Y'));

    ctrl.processApproval();

    EXPECT_EQ(oRepo.findById("ORD-T001")->status, OrderStatus::CONFIRMED);
    EXPECT_EQ(sRepo.findById("S-001")->stock, 100); // 200 - 100
}

// 재고 부족 → PRODUCING, 생산큐 등록
TEST_F(OrderControllerTest, Approval_InsufficientStock_BecomesProducing) {
    addSample("S-001", 30, 0.92, 0.8); // 재고 30, 주문 80 → 부족
    addOrder("ORD-T001", "S-001", 80);

    SampleRepository           sRepo(sPath);
    OrderRepository            oRepo(oPath);
    ProductionLineService      svc;
    NiceMock<MockOrderView>    mockOrderView;
    NiceMock<MockMonitorView>  mockMonitor;
    NiceMock<MockProductionLineView> mockProd;

    OrderController ctrl(sRepo, oRepo, svc, mockOrderView, mockMonitor, mockProd);

    EXPECT_CALL(mockOrderView, showReservedList(_, _)).WillOnce(Return(1));
    EXPECT_CALL(mockOrderView, showApprovalDetail(_, _, _, _, _)).WillOnce(Return('Y'));

    ctrl.processApproval();

    EXPECT_EQ(oRepo.findById("ORD-T001")->status, OrderStatus::PRODUCING);
    EXPECT_FALSE(svc.isEmpty()); // 생산큐에 등록됨
    // 재고 차감 없음 — 예약 방식
    EXPECT_EQ(sRepo.findById("S-001")->stock, 30);
}

// 거절 → REJECTED
TEST_F(OrderControllerTest, Approval_Rejected_BecomesRejected) {
    addSample("S-001", 200);
    addOrder("ORD-T001", "S-001", 100);

    SampleRepository           sRepo(sPath);
    OrderRepository            oRepo(oPath);
    ProductionLineService      svc;
    NiceMock<MockOrderView>    mockOrderView;
    NiceMock<MockMonitorView>  mockMonitor;
    NiceMock<MockProductionLineView> mockProd;

    OrderController ctrl(sRepo, oRepo, svc, mockOrderView, mockMonitor, mockProd);

    EXPECT_CALL(mockOrderView, showReservedList(_, _)).WillOnce(Return(1));
    EXPECT_CALL(mockOrderView, showApprovalDetail(_, _, _, _, _)).WillOnce(Return('R'));

    ctrl.processApproval();

    EXPECT_EQ(oRepo.findById("ORD-T001")->status, OrderStatus::REJECTED);
    // 재고 변화 없음
    EXPECT_EQ(sRepo.findById("S-001")->stock, 200);
}

// CONFIRMED → RELEASED
TEST_F(OrderControllerTest, Release_ConfirmedOrder_BecomesReleased) {
    addSample("S-001", 100);
    addOrder("ORD-T001", "S-001", 50, OrderStatus::CONFIRMED);

    SampleRepository           sRepo(sPath);
    OrderRepository            oRepo(oPath);
    ProductionLineService      svc;
    NiceMock<MockOrderView>    mockOrderView;
    NiceMock<MockMonitorView>  mockMonitor;
    NiceMock<MockProductionLineView> mockProd;

    OrderController ctrl(sRepo, oRepo, svc, mockOrderView, mockMonitor, mockProd);

    EXPECT_CALL(mockOrderView, showConfirmedList(_, _)).WillOnce(Return(1));

    ctrl.processRelease();

    EXPECT_EQ(oRepo.findById("ORD-T001")->status, OrderStatus::RELEASED);
}
