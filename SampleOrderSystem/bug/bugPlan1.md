# Bug Plan 1 — fixrequest1.md 분석 및 수정 계획

## 분석 요약

두 버그 모두 실제 문제이며 각각 **설계 누락**과 **비즈니스 로직 오류**에 해당한다.

---

## Bug 1 — 생산라인 조회 시 PRODUCING 주문이 보이지 않음

### 현상
- 메인 메뉴 "생산대기 N건" 표시 O
- [5] 생산라인 조회 진입 시 "생산라인에 작업이 없습니다." 출력

### 원인 분석

`ProductionLineService`는 **순수 인메모리** 자료구조(`std::queue`)만 사용한다.

```
프로그램 실행 흐름:
  OrderRepository.load()  → orders.json 읽음 → PRODUCING 주문 존재
  ProductionLineService() → 생성자만 실행    → 큐 비어있음 (JSON 미참조)

메인 메뉴 buildSummary():
  orderRepo_.findByStatus(PRODUCING) → 5건 존재 → "생산대기 5건" 표시 ✓

생산라인 조회:
  productionSvc_.isEmpty() → true (큐 비어있음) → "작업이 없습니다" ✗
```

추가 문제: `ProductionTask`에 필요한 `shortage`, `actualProduction`,
`totalTime`, `yieldRate`는 `orders.json`에 저장되지 않아 복원 불가.

### 수정 방향

**Step 1** — `Order` 구조체에 생산 관련 필드 추가 (PRODUCING 상태일 때만 유효)
```cpp
struct Order {
    // ... 기존 필드 ...
    int    prodShortage    = 0;
    int    prodActual      = 0;
    double prodTotalTime   = 0.0;
    double prodYieldRate   = 0.0;
};
```

**Step 2** — `Order::toJson()` / `fromJson()`에 신규 필드 직렬화 추가

**Step 3** — 승인 처리 시(`processApproval`) PRODUCING 전환 시점에 필드 저장
```cpp
order.prodShortage  = shortage;
order.prodActual    = actualProd;
order.prodTotalTime = totalTime;
order.prodYieldRate = sample.yieldRate;
order.status        = OrderStatus::PRODUCING;
```

**Step 4** — `main.cpp` 초기화 코드: PRODUCING 주문 로드 후 큐 복원
```cpp
// 앱 시작 시 PRODUCING 주문 → ProductionTask로 재구성하여 enqueue
auto producing = orderRepo.findByStatus(OrderStatus::PRODUCING);
for (const auto& o : producing) {
    ProductionTask task;
    task.orderId          = o.id;
    task.sampleId         = o.sampleId;
    task.sampleName       = sampleRepo.findById(o.sampleId)->name;
    task.orderQuantity    = o.quantity;
    task.shortage         = o.prodShortage;
    task.actualProduction = o.prodActual;
    task.totalTime        = o.prodTotalTime;
    task.yieldRate        = o.prodYieldRate;
    task.startTime        = time(nullptr); // 재시작 시 처음부터 진행
    productionSvc.enqueue(task);
}
```

---

## Bug 2 — 재고 이중 사용으로 인한 생산 후 재고 음수

### 현상
- 고객 A: 100ea 주문, 재고 50ea → 부족(50ea) → PRODUCING 대기
- 고객 B: 30ea 주문, 재고 여전히 50ea → CONFIRMED, 재고 20ea로 감소
- 고객 A 생산 완료: `newStock = 20 + actualProd - 100` → **음수 발생**

### 원인 분석

현재 코드: PRODUCING 승인 시 재고를 **차감하지 않음**

```cpp
// OrderController::processApproval (현재)
} else {
    order.status = OrderStatus::PRODUCING;
    // ← sample.stock 변경 없음! 타 주문이 동일 재고 사용 가능
    productionSvc_.enqueue(task);
}
```

이후 고객 B 승인 시 stock=50을 그대로 보고 CONFIRMED 처리 →  
고객 A 생산 완료 시 `20 + 62 - 100 = -18` 음수 재고 발생.

### 수정 방향 — "재고 예약(Reservation)" 방식

> 재고는 실제 출하(RELEASED) 전까지 표면상 유지하되,  
> PRODUCING 주문이 선점한 재고는 **예약분**으로 처리하여  
> 후속 주문의 가용 재고 계산 시 제외한다.

**핵심 개념**
```
가용 재고 = 현재 재고 - PRODUCING 주문들의 예약분 합계
예약분    = 승인 당시 이미 있던 재고 = order.quantity - order.prodShortage
```

**Step 1** — `Order` 구조체에 생산 관련 필드 추가 (Bug 1과 공유)
```cpp
// prodShortage 가 있어야 예약분 역산 가능
int    prodShortage  = 0;   // 승인 당시 부족분
int    prodActual    = 0;   // 실 생산량
double prodTotalTime = 0.0; // 총 생산 시간
double prodYieldRate = 0.0; // 수율
```

**Step 2** — 승인 시 가용 재고 기반으로 shortage 재계산
```cpp
// processApproval() 내 shortage 계산 전에 예약분 차감
int reservedStock = 0;
for (const auto& po : orderRepo_.findByStatus(OrderStatus::PRODUCING)) {
    if (po.sampleId == sample.id)
        reservedStock += (po.quantity - po.prodShortage); // 해당 시료의 예약분 합산
}
int availableStock = std::max(0, sample.stock - reservedStock);
int shortage       = order.quantity - availableStock;  // 가용 재고 기준 부족분
```

**Step 3** — PRODUCING 저장 시 order 필드에 실제 사용된 값 기록
```cpp
order.prodShortage  = shortage;
order.prodActual    = actualProd;
order.prodTotalTime = totalTime;
order.prodYieldRate = sample.yieldRate;
order.status        = OrderStatus::PRODUCING;
// sample.stock 변경 없음 — 예약 방식이므로 재고 유지
```

**Step 4** — 생산 완료 공식은 기존 유지 (stock 차감이 없었으므로)
```cpp
// autoCompleteFinished + showProductionLine 모두 동일
sample.stock = sample.stock + task.actualProduction - task.orderQuantity;
// stock(50) + actualProd(62) - orderQty(100) = 12 ✓
```

**Step 5** — 모니터링 표시: 예약분 주석 추가
```
 시료명                 재고       예약       상태     잔여율
 실리콘 웨이퍼          50 ea   (주문예약: 50 ea)   [부족]  ████...
```
- `StockInfo`에 `reservedStock` 필드 추가
- `MonitorView::showStockStats()` / `showOrderStats()`에서 예약분 표시

### 검증

```
초기: stock=50, Order A(100ea)
  reservedStock = 0
  availableStock = 50 - 0 = 50
  shortage = 100 - 50 = 50, actualProd = 62
  PRODUCING, stock 유지=50, 예약분=50

Order B(30ea) 승인:
  reservedStock = 100 - 50 = 50  (A의 예약분)
  availableStock = 50 - 50 = 0
  shortage = 30 - 0 = 30, actualProd = 37
  PRODUCING, stock 유지=50, 예약분=80

모니터링: 50 ea (주문예약: 80 ea) [부족]

Order A 생산 완료:
  newStock = 50 + 62 - 100 = 12 ea ✓

Order B 생산 완료:
  newStock = 12 + 37 - 30 = 19 ea ✓
```

---

## 수정 작업 목록

| # | 파일 | 내용 | Bug |
|---|------|------|-----|
| 1 | `model/Order.h` | `prodShortage`, `prodActual`, `prodTotalTime`, `prodYieldRate` 필드 추가 | 1+2 |
| 2 | `model/Order.cpp` | `fromJson` / `toJson` 신규 필드 직렬화 추가 | 1+2 |
| 3 | `controller/OrderController.cpp` | PRODUCING 승인 시 ① 예약분 반영 가용재고 계산 ② order 필드 저장 | 2 |
| 4 | `view/MonitorView.h` | `StockInfo`에 `reservedStock` 필드 추가 | 2 |
| 5 | `controller/OrderController.cpp` | `showMonitoring()` — StockInfo 생성 시 reservedStock 계산 | 2 |
| 6 | `view/MonitorView.cpp` | 재고 표시에 `(주문예약: Nea)` 주석 추가 | 2 |
| 7 | `main.cpp` | 앱 시작 시 PRODUCING 주문 읽어 ProductionLineService 큐 복원 | 1 |

## 수정 순서

`Order` 구조체 필드 추가(#1, #2)가 Bug 1·2 모두의 기반.  
**#1 → #2 → #3 → #7 (Bug 1 완료) → #4 → #5 → #6 (Bug 2 완료)**
