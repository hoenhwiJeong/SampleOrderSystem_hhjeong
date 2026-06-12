# Phase 3 — Service + Controller 레이어

## 목표

핵심 비즈니스 로직 완성.
View 없이도 모든 주문 처리 시나리오(접수·승인·생산·출고)가 동작하는 상태.

## 상태: ⬜ 대기

---

## 작업 목록

### 3-1. service/ProductionLineService.h/.cpp 구현

**ProductionTask 구조체**
```cpp
struct ProductionTask {
    std::string orderId;
    std::string sampleId;
    std::string sampleName;
    int         orderQuantity;    // 주문 수량
    int         shortage;         // 부족분 = 주문량 - 승인 당시 재고
    int         actualProduction; // 실 생산량 = ceil(shortage / (yieldRate * 0.9))
    double      totalTime;        // 총 생산시간 = avgProductionTime * actualProduction
};
```

**ProductionLineService.h**
```cpp
#pragma once
#include "ProductionTask.h"
#include <queue>
#include <optional>

class ProductionLineService {
public:
    void                          enqueue(const ProductionTask& task);
    std::optional<ProductionTask> currentTask() const;
    std::queue<ProductionTask>    waitingQueue() const;
    bool                          isEmpty() const;
    bool                          hasCurrentTask() const;
    void                          completeCurrentTask();

private:
    std::optional<ProductionTask> current_;
    std::queue<ProductionTask>    queue_;
};
```

**구현 조건**
- [ ] `enqueue()`: `current_` 가 비어있으면 바로 current로 설정, 있으면 `queue_` 에 추가
- [ ] `completeCurrentTask()`: current 제거 후 queue에서 다음 작업을 current로 이동
- [ ] `waitingQueue()`: 대기 큐 복사본 반환 (원본 보호)
- [ ] `isEmpty()`: current도 없고 queue도 비어있으면 true

---

### 3-2. controller/SampleController.h/.cpp 구현

**SampleController.h**
```cpp
#pragma once
#include "../repository/SampleRepository.h"

class SampleController {
public:
    explicit SampleController(SampleRepository& repo);

    void handleMenu();          // 시료 관리 서브 메뉴 루프
    void registerSample();      // 시료 등록
    void listSamples();         // 시료 목록 (5건 페이징)
    void searchSamples();       // 이름 검색

private:
    SampleRepository& repo_;

    bool validateId(const std::string& id);
    bool validateYieldRate(double rate);
    bool validateProductionTime(double time);
};
```

**registerSample() 처리 흐름**
```
1. ID 입력 → 중복 여부 확인
2. 이름 입력
3. 평균 생산시간 입력 (> 0)
4. 수율 입력 (0.0 초과 ~ 1.0 이하)
5. 초기 재고 입력 (≥ 0)
6. 입력 내용 출력 후 [Y/N] 확인
7. Y → repo_.add() 호출
```

**listSamples() 처리 흐름**
```
1. repo_.findAll() 호출
2. 5건씩 출력
3. [N] 다음 페이지 / [0] 뒤로
```

**구현 조건**
- [ ] 잘못된 입력 시 오류 메시지 출력 후 재입력
- [ ] 검색 결과 없을 시 "검색 결과가 없습니다." 출력

---

### 3-3. controller/OrderController.h/.cpp 구현

**OrderController.h**
```cpp
#pragma once
#include "../repository/SampleRepository.h"
#include "../repository/OrderRepository.h"
#include "../service/ProductionLineService.h"

class OrderController {
public:
    OrderController(SampleRepository& sampleRepo,
                    OrderRepository&  orderRepo,
                    ProductionLineService& productionService);

    void placeOrder();          // [메뉴2] 주문 접수
    void processApproval();     // [메뉴3] 주문 승인/거절
    void showMonitoring();      // [메뉴4] 모니터링
    void showProductionLine();  // [메뉴5] 생산라인 조회
    void processRelease();      // [메뉴6] 출고 처리

private:
    SampleRepository&      sampleRepo_;
    OrderRepository&       orderRepo_;
    ProductionLineService& productionService_;

    std::string generateOrderId();
    int         calcActualProduction(int shortage, double yieldRate);
    double      calcTotalTime(double avgTime, int actualProduction);
    std::string stockStatus(const Sample& s);   // "여유"/"부족"/"고갈"
};
```

**generateOrderId() 로직**
```cpp
// 오늘 날짜(YYYYMMDD) 기준 기존 주문 수 + 1 = 순번
// 형식: ORD-YYYYMMDD-NNNN
std::string generateOrderId() {
    // std::chrono 또는 <ctime> 으로 오늘 날짜 획득
    // orderRepo_.countByDate(date) + 1 → 순번
    // 순번 4자리 zero-padding
}
```

**placeOrder() 처리 흐름**
```
1. 시료 ID 입력 → sampleRepo_.findById() 확인
2. 고객명 입력
3. 주문 수량 입력 (> 0)
4. 입력 내용 확인 [Y/N]
5. Y → Order 생성 (status=RESERVED), orderRepo_.add()
6. 완료 메시지 (주문번호, 현재 상태 출력)
```

**processApproval() 처리 흐름**
```
1. orderRepo_.findByStatus(RESERVED) → 목록 출력
2. 목록 없으면 "승인 대기 중인 주문이 없습니다." 출력 후 반환
3. 번호 선택
4. 재고 확인 (shortage = quantity - sample.stock)

   [재고 충분: shortage <= 0]
   → 재고 충분 메시지 출력
   → [Y] 승인: sample.stock -= quantity, order.status = CONFIRMED
   → [N] 거절: order.status = REJECTED

   [재고 부족: shortage > 0]
   → 실 생산량, 총 생산시간 계산 후 메시지 출력
   → [Y] 승인: order.status = PRODUCING, ProductionTask 생성 후 enqueue()
   → [N] 거절: order.status = REJECTED

5. sampleRepo_.update() / orderRepo_.update() 저장
```

**실 생산량 계산**
```cpp
int calcActualProduction(int shortage, double yieldRate) {
    return (int)std::ceil(shortage / (yieldRate * 0.9));
}
```

**showMonitoring() 처리 흐름**
```
1. 서브 메뉴 [1] 주문량 확인 / [2] 재고량 확인

   [주문량 확인]
   → 상태별 건수: RESERVED / CONFIRMED / PRODUCING / RELEASED
   → REJECTED 제외

   [재고량 확인]
   → 시료별 재고 + stockStatus() 표시
```

**stockStatus() 로직**
```cpp
std::string stockStatus(const Sample& s) {
    auto confirmed = orderRepo_.findByStatus(OrderStatus::CONFIRMED);
    int total = 0;
    for (auto& o : confirmed)
        if (o.sampleId == s.id) total += o.quantity;

    if (s.stock == 0)          return "고갈";
    if (s.stock < total)       return "부족";
    return "여유";
}
```

**showProductionLine() 처리 흐름**
```
1. productionService_.currentTask() → 현재 작업 표시
2. productionService_.waitingQueue() → 대기 큐 테이블 출력
3. [C] 선택 시 생산 완료 처리:
   a. currentTask의 orderId로 order 조회
   b. sample.stock = stock + actualProduction - order.quantity
   c. order.status = CONFIRMED
   d. sampleRepo_.update(), orderRepo_.update()
   e. productionService_.completeCurrentTask()
```

**processRelease() 처리 흐름**
```
1. orderRepo_.findByStatus(CONFIRMED) → 목록 출력
2. 목록 없으면 "출고 가능한 주문이 없습니다." 출력 후 반환
3. 번호 선택
4. order.status = RELEASED
5. orderRepo_.update() 저장
6. 완료 메시지 (주문번호, 출고 수량, 처리 일시, 상태 변경)
```

---

### 3-4. vcxproj 소스 파일 등록

- [ ] `service/ProductionLineService.cpp`
- [ ] `controller/SampleController.cpp`
- [ ] `controller/OrderController.cpp`

---

## 완료 기준

| 항목 | 확인 |
|------|------|
| 주문 접수 → RESERVED 상태 저장 확인 | ⬜ |
| 주문 승인(재고 충분) → CONFIRMED + 재고 차감 확인 | ⬜ |
| 주문 승인(재고 부족) → PRODUCING + 생산라인 enqueue 확인 | ⬜ |
| 실 생산량 공식 `ceil(부족분/(수율×0.9))` 계산 정확성 확인 | ⬜ |
| 주문 거절 → REJECTED 상태 저장 확인 | ⬜ |
| 생산 완료 처리 → CONFIRMED + 재고 반영 확인 | ⬜ |
| 재고 상태(여유/부족/고갈) 판단 정확성 확인 | ⬜ |
| 출고 처리 → RELEASED 상태 저장 확인 | ⬜ |
| 생산라인 FIFO 순서 유지 확인 | ⬜ |
| 빌드 성공 (경고 없음) | ⬜ |

---

## 다음 단계

Phase 3 완료 후 → **Phase 4 (View + main.cpp 통합)** 진행
