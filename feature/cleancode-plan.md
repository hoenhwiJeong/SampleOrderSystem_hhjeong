# Clean Code 적용 계획

참조: `feature/cleancode.md`
현재 코드 기준 진단 후, 실현 가능한 항목만 선별.

---

## 적용 판정 요약

| # | 항목 | 근거 조항 | 판정 | 난이도 |
|---|------|----------|------|--------|
| 1 | 매직 넘버 `0.9` 상수화 | §8 하드코딩 금지, §9 매직 넘버 금지 | ✅ 적용 | 쉬움 |
| 2 | 축약 변수명 정리 | §1.2 의미 있는 이름, §1.3 축약어 금지 | ✅ 적용 | 쉬움 |
| 3 | `processApproval()` 함수 분리 | §2.1 단일 책임, §2.2 50라인 이하 | ✅ 적용 | 보통 |
| 4 | `showMonitoring()` 중복 제거 | §2.1 단일 책임 | ✅ 적용 | 쉬움 |
| 5 | 생산 완료 공통 로직 추출 | §2.1 단일 책임 | ✅ 적용 | 쉬움 |
| 6 | WHAT 주석 제거 | §7 코드 설명 주석 금지 | ✅ 적용 | 쉬움 |
| 7 | 함수명 PascalCase | §11 네이밍 규칙 | ❌ 생략 | 매우 어려움 |
| 8 | Repository 인터페이스(DIP) | §4.D 의존성 역전 | ❌ 생략 | 어려움 |
| 9 | Console 출력 추상화 | §5 직접 출력 최소화 | ❌ 생략 | 어려움 |
| 10 | Logger 도입 | §6 예외 로그 기록 | ❌ 생략 | 어려움 |
| 11 | 프로젝트 구조 계층화 | §10 domain/application/infra | ❌ 생략 | 매우 어려움 |

---

## 적용 항목 상세

---

### #1 — 매직 넘버 `0.9` 상수화

**문제** (`BusinessLogic.cpp`)
```cpp
// 0.9 의 의미가 코드에서 불명확
return static_cast<int>(std::ceil(shortage / (yieldRate * 0.9)));
```

**수정** (`BusinessLogic.h`에 상수 추가)
```cpp
namespace BusinessLogic {
    // 공정 오차 10% 안전 마진 (불량률 보정 계수)
    constexpr double DEFECT_RATE_MARGIN = 0.9;

    int calcActualProduction(int shortage, double yieldRate);
    ...
}
```

```cpp
// BusinessLogic.cpp
return static_cast<int>(std::ceil(shortage / (yieldRate * DEFECT_RATE_MARGIN)));
```

**영향 범위**: `BusinessLogic.h`, `BusinessLogic.cpp`

---

### #2 — 축약 변수명 정리

**문제** (`OrderController.cpp` 전체)

| 현재 | 변경 후 | 위치 |
|------|---------|------|
| `sOpt` | `sampleOpt` | `processApproval`, `autoCompleteFinished`, `showProductionLine` |
| `oOpt` | `orderOpt` | `autoCompleteFinished`, `showProductionLine` |
| `actualProd` | `actualProduction` | `processApproval` |
| `sel` | `selectedIndex` | `processApproval`, `processRelease` |
| `c` | `userInput` | `showProductionLine` |
| `in` | `orderInput` | `placeOrder` |

**문제** (`MonitorView.cpp`)

| 현재 | 변경 후 |
|------|---------|
| `pct` | `stockPercent` |
| `s` (for 루프 내) | `stockInfo` |
| `st` | `stockStatus` (OrderController의 `showMonitoring`) |

**영향 범위**: `OrderController.cpp`, `MonitorView.cpp`

---

### #3 — `processApproval()` 함수 분리

**문제**: 현재 약 70라인 (가이드 최대 50라인 초과)

```
processApproval()  ← 70라인, 3가지 책임 혼재
├── 재고 계산 및 부족분 산출
├── 승인 시 재고 충분 처리
└── 승인 시 생산라인 투입 처리
```

**수정**: private 헬퍼 3개 추출

```cpp
// OrderController.h — private 추가
void approveWithSufficientStock(Order& order, Sample& sample);
void approveWithProduction(Order& order, const Sample& sample,
                           int shortage, int actualProduction, double totalTime);
ProductionTask buildProductionTask(const Order& order, const Sample& sample,
                                   int shortage, int actualProduction, double totalTime);
```

```cpp
// processApproval() — 분리 후 약 35라인
void OrderController::processApproval() {
    while (true) {
        auto reserved = orderRepo_.findByStatus(OrderStatus::RESERVED);
        if (reserved.empty()) { orderView_.showNoOrders("..."); return; }

        auto allSamples   = sampleRepo_.findAll();
        int selectedIndex = orderView_.showReservedList(reserved, allSamples);
        if (selectedIndex == 0) return;
        if (selectedIndex < 1 || selectedIndex > (int)reserved.size()) continue;

        Order  order  = reserved[selectedIndex - 1];
        auto   sampleOpt = sampleRepo_.findById(order.sampleId);
        if (!sampleOpt) continue;
        Sample sample = *sampleOpt;

        auto producingOrders  = orderRepo_.findByStatus(OrderStatus::PRODUCING);
        int  reservedStock    = BusinessLogic::calcReservedStock(producingOrders, sample.id);
        int  availableStock   = BusinessLogic::calcAvailableStock(sample.stock, reservedStock);
        int  shortage         = order.quantity - availableStock;
        int  actualProduction = (shortage > 0)
                                    ? BusinessLogic::calcActualProduction(shortage, sample.yieldRate)
                                    : 0;
        double totalTime      = (shortage > 0)
                                    ? BusinessLogic::calcTotalTime(sample.avgProductionTime, actualProduction)
                                    : 0.0;

        char decision = orderView_.showApprovalDetail(sample, order, shortage, actualProduction, totalTime);
        if (decision == '0') continue;

        if (decision == 'Y') {
            if (shortage <= 0) approveWithSufficientStock(order, sample);
            else               approveWithProduction(order, sample, shortage, actualProduction, totalTime);
        } else {
            order.status = OrderStatus::REJECTED;
        }

        orderRepo_.update(order);
        orderView_.showApprovalResult(order);
    }
}
```

**영향 범위**: `OrderController.h`, `OrderController.cpp`

---

### #4 — `showMonitoring()` 중복 제거

**문제**: case 1과 case 2에 동일한 StockInfo 빌드 로직이 완전 중복

```cpp
// 현재: case 1과 case 2에 각각 동일 코드 15라인씩 → 30라인 중복
std::vector<StockInfo> stocks;
for (const auto& s : samples) {
    int confirmedTotal = 0;
    int reservedStock  = 0;
    for (const auto& o : orders) { ... }
    std::string st = ...;
    stocks.push_back({s, st, confirmedTotal, reservedStock});
}
```

**수정**: private 헬퍼로 추출

```cpp
// OrderController.h — private 추가
std::vector<StockInfo> buildStockInfoList(const std::vector<Sample>& samples,
                                          const std::vector<Order>&  orders) const;
```

```cpp
// showMonitoring() — case 1, 2 모두 한 줄로
case 1: {
    auto orders  = orderRepo_.findAll();
    auto samples = sampleRepo_.findAll();
    monitorView_.showOrderStats(orders, buildStockInfoList(samples, orders));
    break;
}
case 2: {
    auto samples = sampleRepo_.findAll();
    auto orders  = orderRepo_.findAll();
    monitorView_.showStockStats(buildStockInfoList(samples, orders));
    break;
}
```

**영향 범위**: `OrderController.h`, `OrderController.cpp`

---

### #5 — 생산 완료 공통 로직 추출

**문제**: `autoCompleteFinished()`와 `showProductionLine()`에 재고 갱신 + 주문 상태 CONFIRMED 처리 코드가 중복

```cpp
// 현재: 두 함수에 각각 존재하는 동일 패턴 10라인
auto sOpt = sampleRepo_.findById(task.sampleId);
if (sOpt) {
    Sample sample = *sOpt;
    sample.stock = BusinessLogic::calcStockAfterProduction(...);
    sampleRepo_.update(sample);
}
auto oOpt = orderRepo_.findById(task.orderId);
if (oOpt) {
    Order order = *oOpt;
    order.status = OrderStatus::CONFIRMED;
    orderRepo_.update(order);
}
```

**수정**: private 헬퍼로 추출

```cpp
// OrderController.h — private 추가
int finalizeProductionTask(const ProductionTask& task); // 반환: 갱신된 재고량
```

**영향 범위**: `OrderController.h`, `OrderController.cpp`

---

### #6 — WHAT 주석 제거

**문제**: 코드가 이미 설명하는 내용을 주석으로 반복

```cpp
// 제거 대상 (코드가 이미 자명)
// 재고 갱신: 현재 재고 + 생산량 - 주문수량
// 주문 상태 → CONFIRMED
// 'R' — 거절
// ── 비공개 유틸 ────

// 유지 대상 (WHY 설명, 비즈니스 의도)
// 재고 차감 없음 — 예약 방식이므로 재고 유지
// 공정 오차 10% 안전 마진 (불량률 보정 계수)
// 생산시간 미확정 — 자동 완료 금지
```

**영향 범위**: `OrderController.cpp`, `BusinessLogic.h`

---

## 생략 항목 사유

| 항목 | 생략 이유 |
|------|----------|
| **함수명 PascalCase** | C++ 표준 및 STL은 camelCase 관례. 전체 리네임 시 영향 범위가 수십 개 파일 전체. 실익 대비 비용 과다 |
| **Repository 인터페이스(DIP)** | 현재 단일 구현체 → 인터페이스 추출 실익 없음. Mock 필요 시 Google Mock으로 직접 모킹 가능 |
| **Console 출력 추상화** | View에서 ConsoleUI 이미 경유. 남은 `std::cout` 직접 호출은 개행·공백 등 포매팅 목적으로 허용 수준 |
| **Logger 도입** | 콘솔 PoC 범위 초과. 별도 인프라 추가 필요 |
| **프로젝트 구조 계층화** | MVC + Repository + Service 구조가 이미 충분. domain/infra 분리는 엔터프라이즈 규모에서 의미 있음 |

---

## 작업 순서

```
#1 매직 넘버     → BusinessLogic.h/.cpp
#6 WHAT 주석    → OrderController.cpp         ← 가장 쉬움, 먼저
#2 변수명 정리  → OrderController.cpp, MonitorView.cpp
#4 중복 제거    → OrderController.h/.cpp      ← #3 전에 해야 충돌 없음
#5 공통 추출    → OrderController.h/.cpp
#3 함수 분리    → OrderController.h/.cpp      ← 마지막 (의존 헬퍼 먼저 완료 후)
```

예상 소요: 파일 6개, 변경량 중간 수준.
기존 동작 변경 없음 — 테스트 35개로 회귀 검증 가능.
