# Phase 4 — View 레이어 + main.cpp 통합

## 목표

사용자 입출력 화면 구현 및 전체 시스템 연결.
완전히 동작하는 콘솔 애플리케이션 완성.

## 상태: ⬜ 대기

---

## 작업 목록

### 4-1. view/MainView.h/.cpp 구현

**MainView.h**
```cpp
#pragma once
#include <string>

struct SystemSummary {
    int    sampleCount;      // 등록 시료 수
    int    totalStock;       // 총 재고
    int    totalOrders;      // 전체 주문 수 (REJECTED 제외)
    int    producingCount;   // 생산라인 대기 수
};

class MainView {
public:
    int           showMenu(const SystemSummary& summary);
    void          showError(const std::string& msg);
    void          showSuccess(const std::string& msg);
    void          pause();   // "계속하려면 Enter를 누르세요..."
};
```

**showMenu() 출력 형식**
```
============================================================
     S-Semi 반도체 시료 생산주문관리 시스템
============================================================
 시스템 현황  2026-06-12 09:32:15

 등록 시료   5종    총 재고   1,640 ea
 전체 주문   8건    생산라인   2건 대기
------------------------------------------------------------
 [1] 시료 관리          [2] 시료 주문
 [3] 주문 승인/거절     [4] 모니터링
 [5] 생산라인 조회      [6] 출고 처리
 [0] 종료
------------------------------------------------------------
선택 > _
```

**구현 조건**
- [ ] 현재 시각 출력 (`<ctime>` 활용)
- [ ] 잘못된 번호 입력 시 "올바른 번호를 입력하세요." 출력 후 재입력
- [ ] `pause()`: 다음 화면으로 넘어가기 전 Enter 대기

---

### 4-2. view/SampleView.h/.cpp 구현

**SampleView.h**
```cpp
#pragma once
#include "../model/Sample.h"
#include <vector>
#include <string>

struct SampleInput {
    std::string id;
    std::string name;
    double      avgProductionTime;
    double      yieldRate;
    int         stock;
};

class SampleView {
public:
    // 입력
    SampleInput   inputSample();
    bool          confirmSampleInput(const SampleInput& input);
    std::string   inputSearchKeyword();

    // 출력
    void showSampleList(const std::vector<Sample>& samples);
    void showSampleDetail(const Sample& s);
    void showNoResult();
    void showRegistered(const std::string& id);
    int  showSubMenu();   // [1]등록 [2]목록 [3]검색 [0]뒤로
};
```

**showSampleList() 출력 형식**
```
등록 시료 목록 (총 5종)
------------------------------------------------------------
ID       시료명                  평균 생산시간   수율    현재 재고
S-001    실리콘 웨이퍼-8인치      0.5 min/ea     0.92    480 ea
S-002    GaN 에피택셜-4인치       0.3 min/ea     0.78    220 ea
S-003    SiC 파워기판-6인치       0.8 min/ea     0.92     30 ea
S-004    포토레지스트-PR7         0.2 min/ea     0.95    910 ea
S-005    산화막 웨이퍼-SiO2       0.6 min/ea     0.88      0 ea
------------------------------------------------------------
[N] 다음 페이지   [0] 뒤로
```

**구현 조건**
- [ ] 5건씩 페이징 출력
- [ ] `confirmSampleInput()`: 입력 내용 재출력 후 [Y/N] 선택

---

### 4-3. view/OrderView.h/.cpp 구현

**OrderView.h**
```cpp
#pragma once
#include "../model/Order.h"
#include "../model/Sample.h"
#include <vector>
#include <string>

struct OrderInput {
    std::string sampleId;
    std::string customerName;
    int         quantity;
};

struct ApprovalInfo {
    int    shortage;
    int    actualProduction;
    double totalTime;
};

class OrderView {
public:
    // 주문 접수
    OrderInput  inputOrder(const std::vector<Sample>& samples);
    bool        confirmOrder(const OrderInput& input, const Sample& s);
    void        showOrderPlaced(const std::string& orderId);

    // 승인/거절
    int         showReservedList(const std::vector<Order>& orders,
                                 const std::vector<Sample>& samples);
    bool        showApprovalSufficient(const Sample& s, const Order& o);
    bool        showApprovalShortage(const Sample& s, const Order& o,
                                     const ApprovalInfo& info);
    void        showApprovalResult(const Order& o, bool approved);
    void        showNoReserved();
};
```

**showReservedList() 출력 형식**
```
승인 대기 중인 예약 목록 (RESERVED)
------------------------------------------------------------
번호    주문번호              고객          시료                  수량
[1]     ORD-20260612-0041    LG이노텍      산화막 웨이퍼-SiO2    300 ea
[2]     ORD-20260612-0042    SK하이닉스    실리콘 웨이퍼-8인치   150 ea
------------------------------------------------------------
승인할 번호 > _
```

**showApprovalShortage() 출력 형식**
```
재고 확인 중...

시료       SiC 파워기판-6인치     현재 재고   30 ea
주문 수량  200 ea                 부족분     170 ea ← 이 수량만큼 생산

재고 부족. 부족분 170 ea 승인하겠습니까? (실생산량 206 ea / 165 min)

[Y] 승인    [N] 주문 거절
선택 > _
```

---

### 4-4. view/MonitorView.h/.cpp 구현

**MonitorView.h**
```cpp
#pragma once
#include "../model/Order.h"
#include "../model/Sample.h"
#include <vector>
#include <string>

struct StockInfo {
    Sample      sample;
    std::string status;    // "여유"/"부족"/"고갈"
    int         confirmedTotal;
};

class MonitorView {
public:
    int  showSubMenu();
    void showOrderStats(const std::vector<Order>& orders);
    void showStockStats(const std::vector<StockInfo>& stocks);
};
```

**showOrderStats() 출력 형식**
```
상태별 주문 현황
------------------------------------------------------------
  RESERVED      3건
  CONFIRMED     8건
  PRODUCING     3건  ← 생산라인 대기
  RELEASED     18건
------------------------------------------------------------
```

**showStockStats() 출력 형식**
```
재고 현황
------------------------------------------------------------
시료명                  재고        상태    잔여율
실리콘 웨이퍼-8인치      480 ea      여유    ████████░░  80%
GaN 에피택셜-4인치       220 ea      여유    ████░░░░░░  44%
SiC 파워기판-6인치        30 ea      부족    █░░░░░░░░░   6%
포토레지스트-PR7         910 ea      여유    █████████░  91%
산화막 웨이퍼-SiO2         0 ea      고갈    ░░░░░░░░░░   0%
------------------------------------------------------------
```

**구현 조건**
- [ ] 상태 표시: 여유(녹색 계열 표현), 부족(주황), 고갈(빨강) — ANSI 또는 텍스트 표기
- [ ] 잔여율 바(█/░) 10칸 기준

---

### 4-5. view/ProductionLineView.h/.cpp 구현

**ProductionLineView.h**
```cpp
#pragma once
#include "../service/ProductionLineService.h"
#include <string>

class ProductionLineView {
public:
    char showProductionLine(
        const std::optional<ProductionTask>& current,
        const std::queue<ProductionTask>&    waiting);
    // 반환: 'C'(완료 처리) 또는 '0'(뒤로)

    void showCompleteResult(const ProductionTask& task,
                            int oldStock, int newStock);
    void showEmpty();
};
```

**showProductionLine() 출력 형식**
```
[5] 생산라인 조회   FIFO 방식
------------------------------------------------------------
생산라인 1개 (단일 라인)    현재 상태: RUNNING
------------------------------------------------------------
[현재 처리 중]
  주문번호  ORD-20260612-0038    시료  SiC 파워기판-6인치
  주문량    80 ea    재고 30 ea → 부족 50 ea → 실생산량 61 ea (수율 0.92 / 49 min)

------------------------------------------------------------
[대기 중인 주문]  (FIFO 순)

  순서   주문번호              시료                   주문량    부족분   실생산량   예상완료
   1     ORD-...-0040    산화막 웨이퍼-SiO2      150 ea   150 ea   190 ea    11:43
   2     ORD-...-0043    SiC 파워기판-6인치      200 ea   170 ea   206 ea    14:28

* 부족분 = 주문량 - 재고,  실생산량 = ceil(부족분 / (수율 × 0.9))
------------------------------------------------------------
[C] 현재 작업 생산 완료 처리    [0] 뒤로
선택 > _
```

**구현 조건**
- [ ] 대기 큐가 비어있으면 "대기 중인 주문이 없습니다." 출력
- [ ] currentTask가 없으면 "현재 생산 중인 작업이 없습니다." 출력
- [ ] 예상 완료 시각: 현재 시각 + 앞 작업들의 totalTime 누적

---

### 4-6. main.cpp 최종 통합

```cpp
#include <iostream>
#include <Windows.h>
#include "repository/SampleRepository.h"
#include "repository/OrderRepository.h"
#include "service/ProductionLineService.h"
#include "controller/SampleController.h"
#include "controller/OrderController.h"
#include "view/MainView.h"

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // 의존성 구성
    SampleRepository      sampleRepo("data/samples.json");
    OrderRepository       orderRepo("data/orders.json");
    ProductionLineService productionService;

    SampleController sampleCtrl(sampleRepo);
    OrderController  orderCtrl(sampleRepo, orderRepo, productionService);
    MainView         mainView;

    while (true) {
        // 시스템 요약 집계
        SystemSummary summary;
        summary.sampleCount    = (int)sampleRepo.findAll().size();
        summary.totalStock     = /* 전체 재고 합산 */;
        summary.totalOrders    = /* REJECTED 제외 주문 수 */;
        summary.producingCount = (int)orderRepo.findByStatus(OrderStatus::PRODUCING).size();

        int choice = mainView.showMenu(summary);
        switch (choice) {
            case 1: sampleCtrl.handleMenu();           break;
            case 2: orderCtrl.placeOrder();             break;
            case 3: orderCtrl.processApproval();        break;
            case 4: orderCtrl.showMonitoring();         break;
            case 5: orderCtrl.showProductionLine();     break;
            case 6: orderCtrl.processRelease();         break;
            case 0:
                std::cout << "시스템을 종료합니다.\n";
                return 0;
        }
    }
}
```

**구현 조건**
- [ ] `totalStock` 계산: `sampleRepo.findAll()` 순회하며 `stock` 합산
- [ ] `totalOrders` 계산: REJECTED 제외 전체 주문 수
- [ ] 메뉴 루프 내 각 Controller 메서드 호출 후 자동으로 메인 메뉴로 복귀

---

### 4-7. vcxproj 소스 파일 등록

- [ ] `view/MainView.cpp`
- [ ] `view/SampleView.cpp`
- [ ] `view/OrderView.cpp`
- [ ] `view/MonitorView.cpp`
- [ ] `view/ProductionLineView.cpp`

---

## 완료 기준

| 항목 | 확인 |
|------|------|
| 메인 메뉴 출력 및 시스템 요약 정상 표시 | ⬜ |
| 시료 등록·목록·검색 화면 정상 동작 | ⬜ |
| 주문 접수 화면 정상 동작 | ⬜ |
| 주문 승인(재고 충분/부족) 화면 정상 동작 | ⬜ |
| 모니터링 주문량·재고량 화면 정상 동작 | ⬜ |
| 생산라인 조회 및 완료 처리 화면 정상 동작 | ⬜ |
| 출고 처리 화면 정상 동작 | ⬜ |
| 전체 플로우 수동 실행 가능 | ⬜ |
| 빌드 성공 (경고 없음) | ⬜ |

---

## 다음 단계

Phase 4 완료 후 → **Phase 5 (통합 테스트 및 검증)** 진행
