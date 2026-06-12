# plan.md — S-Semi 반도체 시료 생산주문관리 시스템 개발 계획

## 전체 Phase 구성

| Phase | 이름 | 핵심 산출물 | 상태 |
|-------|------|-------------|------|
| 1 | 프로젝트 기반 세팅 | 디렉토리 구조, nlohmann/json, JsonHelper, vcxproj | ✅ 완료 |
| 2 | Model + Repository | Sample/Order 모델, SampleRepository, OrderRepository | ✅ 완료 |
| 3 | Service + Controller | ProductionLineService, SampleController, OrderController | ✅ 완료 |
| 4 | View + main.cpp 통합 | 전체 View 파일, main.cpp 연결 | ✅ 완료 |
| 5 | 통합 테스트 및 검증 | 전체 플로우 테스트, 예외 처리 확인, 최종 빌드 | ✅ 완료 |

---

## Phase 1 — 프로젝트 기반 세팅

**목표**: 빌드 가능한 빈 프로젝트 골격 완성. 이후 모든 Phase의 기반.

### 작업 목록

- [ ] 1-1. 디렉토리 구조 생성
  - `model/`, `repository/`, `controller/`, `service/`, `view/`, `util/`, `nlohmann/`, `data/`
- [ ] 1-2. `nlohmann/json.hpp` 추가 (v3.11.3 단일 헤더)
- [ ] 1-3. `util/JsonHelper.h/.cpp` 구현
  - `static nlohmann::json Load(const std::string& path)`
  - `static void Save(const std::string& path, const nlohmann::json& j)`
  - 파일 없을 시 빈 배열 반환
- [ ] 1-4. `SampleOrderSystem.vcxproj` 업데이트
  - C++20 표준, x64, 모든 소스 파일 등록
  - `SetConsoleOutputCP(CP_UTF8)` + `SetConsoleCP(CP_UTF8)` 적용 확인
- [ ] 1-5. `main.cpp` 최소 뼈대 작성 (빌드 확인용)
- [ ] 1-6. `data/samples.json`, `data/orders.json` 빈 파일 생성

### 완료 기준
- UI 느낌을 알려줄 수 있도록 사용될 UI console 접근 가능하도록 만들어 놓는다. consoleUI.md 를 참조
- `msbuild` 빌드 성공 (경고 없음)
- 실행 시 "반도체 시료 생산주문관리 시스템" 텍스트 출력
- `data/` 폴더 내 JSON 파일 읽기/쓰기 테스트 통과

---

## Phase 2 — Model + Repository 레이어

**목표**: 데이터 구조 정의 및 JSON 파일 영속성 구현 완성.

### 작업 목록

- [ ] 2-1. `model/Sample.h/.cpp` 구현
  ```cpp
  struct Sample {
      std::string id;
      std::string name;
      double avgProductionTime;
      double yieldRate;
      int stock;

      static Sample fromJson(const nlohmann::json& j);
      nlohmann::json toJson() const;
  };
  ```

- [ ] 2-2. `model/Order.h/.cpp` 구현
  ```cpp
  enum class OrderStatus { RESERVED, PRODUCING, CONFIRMED, RELEASED, REJECTED };

  struct Order {
      std::string id;           // ORD-YYYYMMDD-NNNN
      std::string sampleId;
      std::string customerName;
      int quantity;
      OrderStatus status;

      static Order fromJson(const nlohmann::json& j);
      nlohmann::json toJson() const;
      static std::string statusToString(OrderStatus s);
      static OrderStatus statusFromString(const std::string& s);
  };
  ```

- [ ] 2-3. `repository/SampleRepository.h/.cpp` 구현
  - `void add(const Sample& s)` — 중복 ID 시 `std::runtime_error`
  - `std::vector<Sample> findAll()`
  - `std::optional<Sample> findById(const std::string& id)`
  - `std::vector<Sample> findByName(const std::string& keyword)` — 부분 문자열
  - `void update(const Sample& s)`
  - `private: void load()` / `void save()`

- [ ] 2-4. `repository/OrderRepository.h/.cpp` 구현
  - `void add(const Order& o)`
  - `std::vector<Order> findAll()`
  - `std::optional<Order> findById(const std::string& id)`
  - `std::vector<Order> findByStatus(OrderStatus status)`
  - `void update(const Order& o)`
  - `int countByDate(const std::string& date)` — 주문 ID 순번 생성용
  - `private: void load()` / `void save()`

### 완료 기준

- `samples.json` / `orders.json` 읽기·쓰기 정상 동작
- Sample·Order 직렬화/역직렬화 왕복 테스트 (toJson → fromJson 값 일치)
- 중복 ID 삽입 시 예외 발생 확인

---

## Phase 3 — Service + Controller 레이어

**목표**: 핵심 비즈니스 로직 완성. View 없이 로직만으로 모든 시나리오 처리 가능.

### 작업 목록

- [ ] 3-1. `service/ProductionLineService.h/.cpp` 구현

  ```cpp
  struct ProductionTask {
      std::string orderId;
      std::string sampleId;
      std::string sampleName;
      int orderQuantity;
      int shortage;
      int actualProduction;   // ceil(shortage / (yieldRate * 0.9))
      double totalTime;       // avgProductionTime * actualProduction
  };

  class ProductionLineService {
  public:
      void enqueue(const ProductionTask& task);
      std::optional<ProductionTask> currentTask() const;  // 현재 처리 중
      std::queue<ProductionTask> waitingQueue() const;    // 대기 큐 복사본
      bool isEmpty() const;
      void completeCurrentTask();  // 큐에서 제거, 다음 작업으로 이동
  };
  ```

- [ ] 3-2. `controller/SampleController.h/.cpp` 구현
  - `void registerSample()` — 입력 검증 + `SampleRepository::add()`
  - `void listSamples()` — 페이징(5건)
  - `void searchSamples()` — 이름 검색

- [ ] 3-3. `controller/OrderController.h/.cpp` 구현
  - `void placeOrder()` — 주문 접수 (RESERVED), 주문 ID 자동 생성
  - `void processApproval()` — 승인/거절 처리
    - 재고 충분 → CONFIRMED + 재고 차감
    - 재고 부족 → PRODUCING + `ProductionLineService::enqueue()`
  - `void processRelease()` — 출고 처리 (CONFIRMED → RELEASED)
  - `void completeProduction()` — 생산 완료 (PRODUCING → CONFIRMED + 재고 반영)
  - `void showMonitoring()` — 상태별 주문 수 + 재고 현황
  - `void showProductionLine()` — 생산라인 현황

  **주문 ID 생성 로직:**
  ```cpp
  // ORD-YYYYMMDD-NNNN
  // 오늘 날짜 기준 기존 주문 수 + 1 = 순번
  std::string generateOrderId();
  ```

  **승인 시 실 생산량 계산:**
  ```cpp
  int shortage = order.quantity - sample.stock;
  int actualProduction = (int)std::ceil(shortage / (sample.yieldRate * 0.9));
  double totalTime = sample.avgProductionTime * actualProduction;
  ```

  **생산 완료 후 재고 반영:**
  ```cpp
  sample.stock = sample.stock + task.actualProduction - order.quantity;
  ```

### 완료 기준

- 주문 접수 → 승인(재고 충분) → 출고 흐름 동작 확인
- 주문 접수 → 승인(재고 부족) → 생산라인 큐 등록 확인
- 생산 완료 처리 후 재고 수량 정확히 반영 확인
- 재고 상태(여유/부족/고갈) 판단 로직 확인

---

## Phase 4 — View 레이어 + main.cpp 통합

**목표**: 사용자 화면(UI) 구현 및 전체 시스템 연결. 완전 동작하는 애플리케이션 완성.

### 작업 목록

- [ ] 4-1. `view/MainView.h/.cpp` 구현
  - ASCII 타이틀 배너 출력
  - 시스템 현황 요약 (시료 수, 총 재고, 전체 주문, 생산라인 대기)
  - 메뉴 번호 입력 및 반환

- [ ] 4-2. `view/SampleView.h/.cpp` 구현
  - 시료 등록 입력 화면
  - 시료 목록 출력 (테이블, 5건 페이징)
  - 시료 검색 결과 출력

- [ ] 4-3. `view/OrderView.h/.cpp` 구현
  - 주문 접수 입력 화면
  - 주문 완료 확인 화면 (주문번호, 상태 표시)
  - RESERVED 주문 목록 + 승인/거절 선택 화면
  - 승인 결과 화면 (재고 충분/부족 분기 메시지)

- [ ] 4-4. `view/MonitorView.h/.cpp` 구현
  - 상태별 주문 건수 표시
  - 시료별 재고 현황 테이블 (여유/부족/고갈 상태 표시)

- [ ] 4-5. `view/ProductionLineView.h/.cpp` 구현
  - 현재 처리 중 작업 상세 표시
  - 대기 큐 테이블 출력 (FIFO 순)
  - [C] 생산 완료 처리 / [0] 뒤로 선택

- [ ] 4-6. `main.cpp` 최종 통합
  ```cpp
  int main() {
      SetConsoleOutputCP(CP_UTF8);
      SetConsoleCP(CP_UTF8);

      // Repository, Service, Controller, View 인스턴스 생성 및 의존성 주입
      // 메인 루프: MainView 메뉴 → 각 Controller 호출
      while (true) {
          int choice = mainView.showMenu();
          switch (choice) {
              case 1: sampleController.handleMenu(); break;
              case 2: orderController.placeOrder(); break;
              case 3: orderController.processApproval(); break;
              case 4: orderController.showMonitoring(); break;
              case 5: orderController.showProductionLine(); break;
              case 6: orderController.processRelease(); break;
              case 0: return 0;
          }
      }
  }
  ```

### 완료 기준

- 모든 메뉴 진입 및 출력 정상 동작
- 전체 플로우 수동 실행 가능 (주문 접수 → 승인 → 생산 완료 → 출고)
- 빌드 경고 없음

---

## Phase 5 — 통합 테스트 및 검증

**목표**: 전체 시나리오 검증, 엣지 케이스 확인, 최종 제출 준비.

### 테스트 시나리오

- [ ] 5-1. **정상 플로우 — 재고 충분**
  1. 시료 등록 (S-001, 재고 100)
  2. 주문 접수 (수량 50)
  3. 주문 승인 → CONFIRMED, 재고 50 확인
  4. 출고 처리 → RELEASED 확인

- [ ] 5-2. **정상 플로우 — 재고 부족**
  1. 시료 등록 (S-002, 재고 30)
  2. 주문 접수 (수량 100)
  3. 주문 승인 → PRODUCING, 생산라인 등록 확인
  4. 생산라인 조회 → 실 생산량/총 생산시간 계산 확인
  5. 생산 완료 처리 → CONFIRMED, 재고 반영 확인
  6. 출고 처리 → RELEASED 확인

- [ ] 5-3. **주문 거절 플로우**
  1. 주문 접수 → 승인 화면에서 [N] 선택
  2. 상태 REJECTED 확인
  3. 모니터링에서 REJECTED 미표시 확인

- [ ] 5-4. **생산라인 FIFO 검증**
  1. 재고 부족 주문 3건 연속 승인
  2. 생산라인 대기 큐 순서 확인 (접수 순)
  3. 생산 완료 처리 반복 → 순서대로 처리 확인

- [ ] 5-5. **모니터링 재고 상태 검증**
  - 재고 0 → 고갈
  - 재고 < CONFIRMED 주문 합산량 → 부족
  - 그 외 → 여유

- [ ] 5-6. **데이터 영속성 검증**
  1. 주문 접수 후 프로그램 종료
  2. 재실행 시 데이터 유지 확인

- [ ] 5-7. **예외 처리 검증**
  - 존재하지 않는 시료 ID로 주문 시도
  - 중복 시료 ID 등록 시도
  - 수량 0 또는 음수 입력
  - 잘못된 메뉴 번호 입력
  - RESERVED/CONFIRMED 주문 없는 상태에서 해당 메뉴 진입

- [ ] 5-8. **최종 빌드 및 제출 준비**
  - Release 빌드 확인
  - `data/` 폴더 내 JSON 초기 상태 확인
  - README 또는 CLAUDE.md 최종 검토
  - GitHub Push 및 Repository Public 설정 확인

### 완료 기준

- 5-1 ~ 5-7 시나리오 전부 통과
- Release x64 빌드 성공
- GitHub Repository Public 상태 확인

---

## 진행 현황 요약

```
Phase 1  [프로젝트 기반 세팅]    ✅ 완료
Phase 2  [Model + Repository]   ✅ 완료
Phase 3  [Service + Controller] ✅ 완료
Phase 4  [View + main.cpp]      ✅ 완료
Phase 5  [통합 테스트 및 검증]   ✅ 완료
```
