# Feature Plan — BusinessLogic 유틸 분리

## 목적

`OrderController`의 private 계산 메서드와 재고 예약 로직을  
`util/BusinessLogic.h/.cpp` 로 꺼내어, 테스트 하네스에서  
View·Repository 의존성 없이 단독으로 검증할 수 있도록 한다.

---

## 현재 문제

비즈니스 로직이 Controller 내부에 묻혀 있어 테스트 불가.

```
OrderController (private)
├── calcActualProduction(shortage, yieldRate)   ← 핵심 공식
├── calcTotalTime(avgTime, actualProd)           ← 핵심 공식
└── processApproval() 내부 인라인 코드
    ├── reservedStock 합산                       ← Bug2 핵심 로직
    ├── availableStock 계산                      ← Bug2 핵심 로직
    └── shortage = quantity - availableStock     ← Bug2 핵심 로직
```

---

## 분리 대상 함수

### util/BusinessLogic.h

```cpp
#pragma once
#include "../model/Order.h"
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

namespace BusinessLogic {

    // 실 생산량: ceil(부족분 / (수율 × 0.9))
    int calcActualProduction(int shortage, double yieldRate);

    // 총 생산시간: 평균생산시간(min/ea) × 실 생산량
    double calcTotalTime(double avgProductionTime, int actualProduction);

    // PRODUCING 주문들이 선점한 재고 합계
    // 예약분 = quantity - prodShortage (승인 당시 있던 재고)
    int calcReservedStock(const std::vector<Order>& orders,
                          const std::string& sampleId);

    // 가용 재고: max(0, 현재 재고 - 예약분)
    int calcAvailableStock(int currentStock, int reservedStock);

    // 재고 부족 여부: quantity > availableStock
    bool isShortage(int quantity, int availableStock);

    // 생산 완료 후 재고: 현재재고 + 실생산량 - 주문수량
    int calcStockAfterProduction(int currentStock,
                                 int actualProduction,
                                 int orderQuantity);

}
```

---

## 작업 목록

| # | 작업 | 파일 | 내용 |
|---|------|------|------|
| 1 | 파일 생성 | `util/BusinessLogic.h` | 위 선언 작성 |
| 2 | 파일 생성 | `util/BusinessLogic.cpp` | 각 함수 구현 |
| 3 | vcxproj 등록 | `SampleOrderSystem.vcxproj` | BusinessLogic.h/.cpp 추가 |
| 4 | Controller 교체 | `OrderController.cpp` | private 메서드 → BusinessLogic:: 호출로 교체 |
| 5 | private 메서드 제거 | `OrderController.h/.cpp` | `calcActualProduction`, `calcTotalTime` 제거 |
| 6 | 빌드 확인 | — | 기존 동작 동일한지 검증 |

---

## 교체 전후 비교

### Before (OrderController.cpp)

```cpp
// private 구현
int OrderController::calcActualProduction(int shortage, double yieldRate) {
    return static_cast<int>(std::ceil(shortage / (yieldRate * 0.9)));
}
double OrderController::calcTotalTime(double avgTime, int actualProduction) {
    return avgTime * actualProduction;
}

// processApproval() 내부 인라인
int reservedStock = 0;
for (const auto& po : orderRepo_.findByStatus(OrderStatus::PRODUCING)) {
    if (po.sampleId == sample.id)
        reservedStock += (po.quantity - po.prodShortage);
}
int availableStock = std::max(0, sample.stock - reservedStock);
int shortage       = order.quantity - availableStock;
```

### After (OrderController.cpp)

```cpp
#include "../util/BusinessLogic.h"

// processApproval() 내부
auto producing     = orderRepo_.findByStatus(OrderStatus::PRODUCING);
int reservedStock  = BusinessLogic::calcReservedStock(producing, sample.id);
int availableStock = BusinessLogic::calcAvailableStock(sample.stock, reservedStock);
int shortage       = order.quantity - availableStock;

if (shortage > 0) {
    actualProd = BusinessLogic::calcActualProduction(shortage, sample.yieldRate);
    totalTime  = BusinessLogic::calcTotalTime(sample.avgProductionTime, actualProd);
}

// autoCompleteFinished() 내부
int newStock = BusinessLogic::calcStockAfterProduction(
    sample.stock, task.actualProduction, task.orderQuantity);
```

---

## 기대 효과

- 테스트 하네스에서 `#include "util/BusinessLogic.h"` 한 줄로 전체 공식 검증 가능
- CLAUDE.md 명세 예시값 그대로 단위 테스트에 활용 가능

```
// CLAUDE.md 검증 예시
// SiC 파워기판, 재고 30ea, 주문 80ea
// 부족분=50, 실생산량=ceil(50/(0.92×0.9))=61, 총시간=0.8×61=48.8
```

- Controller가 얇아져 이후 기능 추가 시 로직 중복 방지
