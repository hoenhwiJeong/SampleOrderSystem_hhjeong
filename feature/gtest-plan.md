# Feature Plan — Google Test / GMock 테스트 하네스 도입

## 목적

Visual Studio 솔루션에 `SampleOrderSystem.Tests` 프로젝트를 추가하여  
핵심 비즈니스 로직·모델 직렬화·서비스 레이어를 자동화 단위 테스트로 검증한다.

---

## 전제 조건

- GMock 설치 완료 (사용자 확인)
- `businesslogic-plan.md` 의 BusinessLogic 분리가 **선행** 완료되어야 함
  - 분리 전에도 테스트는 가능하지만, Controller private 메서드는 직접 테스트 불가

---

## 프로젝트 구조 (추가 후)

```
C:\PoCDev\SampleOrderSystem\
├── SampleOrderSystem.slnx
├── SampleOrderSystem/              ← 기존 (변경 없음)
│   └── ...
└── SampleOrderSystem.Tests/        ← 신규
    ├── SampleOrderSystem.Tests.vcxproj
    ├── test_main.cpp               ← gtest entry point
    ├── test_order_model.cpp        ← Order 직렬화 테스트
    ├── test_business_logic.cpp     ← 핵심 공식 테스트
    ├── test_production_service.cpp ← ProductionLineService 테스트
    └── test_repository.cpp         ← Repository CRUD 테스트
```

---

## 작업 목록

| # | 작업 | 내용 |
|---|------|------|
| 1 | 테스트 프로젝트 생성 | VS에서 새 프로젝트 → Google Test → `SampleOrderSystem.Tests` |
| 2 | vcxproj 설정 | Include 경로에 기존 프로젝트 src 디렉토리 추가 |
| 3 | 프로젝트 참조 추가 | Tests 프로젝트 → SampleOrderSystem 프로젝트 참조 |
| 4 | test_main.cpp 작성 | `::testing::InitGoogleTest` 진입점 |
| 5 | 테스트 파일 4종 작성 | 아래 상세 참조 |
| 6 | 빌드 및 실행 확인 | `Ctrl+R, A` 또는 테스트 탐색기 실행 |

---

## vcxproj 설정 포인트

```xml
<!-- SampleOrderSystem.Tests.vcxproj 에 추가 필요 -->
<ItemDefinitionGroup>
  <ClCompile>
    <AdditionalIncludeDirectories>
      $(SolutionDir)SampleOrderSystem;   <!-- 기존 소스 경로 -->
      %(AdditionalIncludeDirectories)
    </AdditionalIncludeDirectories>
  </ClCompile>
</ItemDefinitionGroup>
```

> 기존 프로젝트의 `.cpp` 파일들을 직접 포함하거나,  
> 프로젝트 참조(Project Reference)로 링크하는 방식 중 하나 선택.

---

## 테스트 파일별 상세

### test_main.cpp

```cpp
#include <gtest/gtest.h>

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
```

---

### test_order_model.cpp — Order 직렬화 검증

```cpp
#include <gtest/gtest.h>
#include "model/Order.h"

// toJson → fromJson 왕복 후 필드 일치
TEST(OrderModel, SerializeDeserializeRoundTrip) {
    Order o;
    o.id           = "ORD-20260612-0001";
    o.sampleId     = "S-001";
    o.customerName = "삼성전자";
    o.quantity     = 100;
    o.status       = OrderStatus::RESERVED;

    auto json    = o.toJson();
    auto rebuilt = Order::fromJson(json);

    EXPECT_EQ(rebuilt.id,           o.id);
    EXPECT_EQ(rebuilt.customerName, o.customerName);
    EXPECT_EQ(rebuilt.quantity,     o.quantity);
    EXPECT_EQ(rebuilt.status,       OrderStatus::RESERVED);
}

// PRODUCING 상태: prod 필드 직렬화
TEST(OrderModel, ProducingFieldsSerialized) {
    Order o;
    o.status       = OrderStatus::PRODUCING;
    o.prodShortage  = 50;
    o.prodActual    = 61;
    o.prodTotalTime = 48.8;
    o.prodYieldRate = 0.92;

    auto json    = o.toJson();
    auto rebuilt = Order::fromJson(json);

    EXPECT_EQ(rebuilt.prodShortage,  50);
    EXPECT_EQ(rebuilt.prodActual,    61);
    EXPECT_DOUBLE_EQ(rebuilt.prodTotalTime, 48.8);
}

// CONFIRMED 상태: prod 필드 직렬화 안 됨 → 기본값 유지
TEST(OrderModel, NonProducingFieldsNotSerialized) {
    Order o;
    o.status       = OrderStatus::CONFIRMED;
    o.prodShortage  = 50; // 저장되어선 안 됨

    auto json    = o.toJson();
    auto rebuilt = Order::fromJson(json);

    EXPECT_EQ(rebuilt.prodShortage, 0);
}
```

---

### test_business_logic.cpp — 핵심 공식 검증

```cpp
#include <gtest/gtest.h>
#include "util/BusinessLogic.h"

// CLAUDE.md 명세 예시: SiC 파워기판 (수율 0.92, 부족 50ea)
TEST(BusinessLogic, CalcActualProduction_ClaudeSpec) {
    // ceil(50 / (0.92 * 0.9)) = ceil(60.4) = 61
    EXPECT_EQ(BusinessLogic::calcActualProduction(50, 0.92), 61);
}

TEST(BusinessLogic, CalcTotalTime_ClaudeSpec) {
    // 0.8 min/ea × 61 = 48.8 min
    EXPECT_DOUBLE_EQ(BusinessLogic::calcTotalTime(0.8, 61), 48.8);
}

TEST(BusinessLogic, StockAfterProduction_ClaudeSpec) {
    // 30 + 61 - 80 = 11
    EXPECT_EQ(BusinessLogic::calcStockAfterProduction(30, 61, 80), 11);
}

// Bug2 시나리오: 재고 50, A(100ea)→PRODUCING, B(30ea) 승인 시
TEST(BusinessLogic, ReservationScenario_Bug2) {
    std::vector<Order> orders;
    Order a;
    a.sampleId     = "S-001";
    a.quantity     = 100;
    a.prodShortage = 50;  // 예약분 = 100 - 50 = 50
    a.status       = OrderStatus::PRODUCING;
    orders.push_back(a);

    int reserved  = BusinessLogic::calcReservedStock(orders, "S-001");
    int available = BusinessLogic::calcAvailableStock(50, reserved);

    EXPECT_EQ(reserved,  50);  // A가 선점한 재고
    EXPECT_EQ(available,  0);  // B가 사용 가능한 재고 없음
}

TEST(BusinessLogic, AvailableStockNotNegative) {
    // 예약분이 현재 재고보다 커도 0 이하로 안 내려감
    EXPECT_EQ(BusinessLogic::calcAvailableStock(10, 50), 0);
}
```

---

### test_production_service.cpp — ProductionLineService 검증

```cpp
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

// 첫 enqueue → current에 즉시 투입
TEST(ProductionLineService, FirstEnqueueBecomesCurrentTask) {
    ProductionLineService svc;
    svc.enqueue(makeTask("ORD-001"));

    EXPECT_FALSE(svc.isEmpty());
    EXPECT_TRUE(svc.hasCurrentTask());
    EXPECT_EQ(svc.currentTask()->orderId, "ORD-001");
    EXPECT_TRUE(svc.waitingQueue().empty());
}

// 두 번째 enqueue → 대기 큐에 쌓임
TEST(ProductionLineService, SecondEnqueueGoesToQueue) {
    ProductionLineService svc;
    svc.enqueue(makeTask("ORD-001"));
    svc.enqueue(makeTask("ORD-002"));

    EXPECT_EQ(svc.waitingQueue().size(), 1u);
    EXPECT_EQ(svc.waitingQueue().front().orderId, "ORD-002");
}

// FIFO 순서 보장
TEST(ProductionLineService, FifoOrderPreserved) {
    ProductionLineService svc;
    svc.enqueue(makeTask("ORD-001"));
    svc.enqueue(makeTask("ORD-002"));
    svc.enqueue(makeTask("ORD-003"));

    svc.completeCurrentTask();
    EXPECT_EQ(svc.currentTask()->orderId, "ORD-002");

    svc.completeCurrentTask();
    EXPECT_EQ(svc.currentTask()->orderId, "ORD-003");

    svc.completeCurrentTask();
    EXPECT_TRUE(svc.isEmpty());
}

// completeCurrentTask 후 startTime 갱신 여부
TEST(ProductionLineService, StartTimeUpdatedOnPromotion) {
    ProductionLineService svc;
    svc.enqueue(makeTask("ORD-001"));
    svc.enqueue(makeTask("ORD-002"));

    svc.completeCurrentTask();
    EXPECT_GT(svc.currentTask()->startTime, 0);
}
```

---

### test_repository.cpp — Repository CRUD 검증

```cpp
#include <gtest/gtest.h>
#include "repository/SampleRepository.h"
#include <filesystem>

class SampleRepositoryTest : public ::testing::Test {
protected:
    const std::string path = "test_samples_tmp.json";

    void SetUp() override {
        // 테스트 전 파일 초기화
        std::filesystem::remove(path);
    }
    void TearDown() override {
        std::filesystem::remove(path);
    }
};

TEST_F(SampleRepositoryTest, AddAndFindById) {
    SampleRepository repo(path);
    Sample s;
    s.id = "S-T01"; s.name = "테스트시료";
    s.avgProductionTime = 0.5; s.yieldRate = 0.9; s.stock = 100;

    repo.add(s);
    auto found = repo.findById("S-T01");

    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name, "테스트시료");
    EXPECT_EQ(found->stock, 100);
}

TEST_F(SampleRepositoryTest, DuplicateIdThrows) {
    SampleRepository repo(path);
    Sample s; s.id = "S-T01"; s.name = "A";

    repo.add(s);
    EXPECT_THROW(repo.add(s), std::runtime_error);
}

TEST_F(SampleRepositoryTest, UpdatePersists) {
    SampleRepository repo(path);
    Sample s; s.id = "S-T01"; s.name = "A"; s.stock = 100;
    repo.add(s);

    s.stock = 200;
    repo.update(s);

    // 새 인스턴스로 재로드해서 영속성 확인
    SampleRepository repo2(path);
    EXPECT_EQ(repo2.findById("S-T01")->stock, 200);
}
```

---

## 실행 방법

```
Visual Studio → 테스트 탐색기 (Ctrl+E, T) → 모두 실행
또는
Developer Command Prompt:
  vstest.console x64\Debug\SampleOrderSystem.Tests.exe
```

---

## 우선순위

| 우선순위 | 파일 | 이유 |
|---|---|---|
| 1순위 | test_business_logic.cpp | CLAUDE.md 명세값 검증 + Bug2 재현 |
| 2순위 | test_production_service.cpp | FIFO 핵심 동작 보증 |
| 3순위 | test_order_model.cpp | JSON 마이그레이션 안전망 |
| 4순위 | test_repository.cpp | 영속성 회귀 방지 |
