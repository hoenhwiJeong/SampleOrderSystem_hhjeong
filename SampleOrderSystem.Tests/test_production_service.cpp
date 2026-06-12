#include <gtest/gtest.h>
#include "service/ProductionLineService.h"

static ProductionTask makeTask(const std::string& id, double totalTime = 30.0) {
    ProductionTask t;
    t.orderId          = id;
    t.sampleId         = "S-001";
    t.sampleName       = "테스트시료";
    t.orderQuantity    = 10;
    t.shortage         = 5;
    t.actualProduction = 6;
    t.totalTime        = totalTime;
    t.yieldRate        = 0.92;
    return t;
}

// ── 초기 상태 ─────────────────────────────────────────────────────────────

TEST(ProductionLineService, InitiallyEmpty) {
    ProductionLineService svc;
    EXPECT_TRUE(svc.isEmpty());
    EXPECT_FALSE(svc.hasCurrentTask());
    EXPECT_TRUE(svc.waitingQueue().empty());
}

// ── enqueue ───────────────────────────────────────────────────────────────

// 첫 enqueue → current에 즉시 투입, startTime 설정됨
TEST(ProductionLineService, FirstEnqueueBecomesCurrentTask) {
    ProductionLineService svc;
    svc.enqueue(makeTask("ORD-001"));

    ASSERT_TRUE(svc.hasCurrentTask());
    EXPECT_EQ(svc.currentTask()->orderId, "ORD-001");
    EXPECT_GT(svc.currentTask()->startTime, 0);
    EXPECT_TRUE(svc.waitingQueue().empty());
    EXPECT_FALSE(svc.isEmpty());
}

// 두 번째 enqueue → 대기 큐에 적재
TEST(ProductionLineService, SecondEnqueueGoesToWaitingQueue) {
    ProductionLineService svc;
    svc.enqueue(makeTask("ORD-001"));
    svc.enqueue(makeTask("ORD-002"));

    EXPECT_EQ(svc.waitingQueue().size(), 1u);
    EXPECT_EQ(svc.waitingQueue().front().orderId, "ORD-002");
}

// ── FIFO 순서 보장 ────────────────────────────────────────────────────────

TEST(ProductionLineService, FifoOrderPreserved) {
    ProductionLineService svc;
    svc.enqueue(makeTask("ORD-001"));
    svc.enqueue(makeTask("ORD-002"));
    svc.enqueue(makeTask("ORD-003"));

    EXPECT_EQ(svc.currentTask()->orderId, "ORD-001");

    svc.completeCurrentTask();
    EXPECT_EQ(svc.currentTask()->orderId, "ORD-002");

    svc.completeCurrentTask();
    EXPECT_EQ(svc.currentTask()->orderId, "ORD-003");

    svc.completeCurrentTask();
    EXPECT_TRUE(svc.isEmpty());
}

// ── completeCurrentTask ───────────────────────────────────────────────────

// 완료 후 다음 작업이 current로 승격되고 startTime이 갱신됨
TEST(ProductionLineService, CompletionPromotesNextTask) {
    ProductionLineService svc;
    svc.enqueue(makeTask("ORD-001"));
    svc.enqueue(makeTask("ORD-002"));

    svc.completeCurrentTask();

    ASSERT_TRUE(svc.hasCurrentTask());
    EXPECT_EQ(svc.currentTask()->orderId, "ORD-002");
    EXPECT_GT(svc.currentTask()->startTime, 0);
}

// 단일 작업 완료 → isEmpty
TEST(ProductionLineService, CompleteSingleTaskBecomesEmpty) {
    ProductionLineService svc;
    svc.enqueue(makeTask("ORD-001"));
    svc.completeCurrentTask();

    EXPECT_TRUE(svc.isEmpty());
    EXPECT_FALSE(svc.hasCurrentTask());
}

// ── waitingQueue 내용 확인 ────────────────────────────────────────────────

TEST(ProductionLineService, WaitingQueueCountMatchesEnqueued) {
    ProductionLineService svc;
    svc.enqueue(makeTask("ORD-001")); // current
    svc.enqueue(makeTask("ORD-002")); // waiting
    svc.enqueue(makeTask("ORD-003")); // waiting

    EXPECT_EQ(svc.waitingQueue().size(), 2u);
}
