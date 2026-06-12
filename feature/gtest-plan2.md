# gtest-plan2.md — 테스트 커버리지 확장 (Phase 2)

## 목표

Phase 1(35개)에서 미커버된 영역을 커버한다.
핵심은 세 가지: **Sample 모델 직렬화**, **Controller 비즈니스 분기**, **재고 상태 판단 로직**.

---

## Phase 1 커버리지 현황 (35개)

| 파일 | 개수 | 커버 범위 |
|------|------|-----------|
| test_business_logic.cpp | 13 | BusinessLogic 순수 함수 (공식, Bug2 시나리오) |
| test_production_service.cpp | 7 | ProductionLineService FIFO 동작 |
| test_order_model.cpp | 5 | Order JSON 직렬화·역직렬화 |
| test_repository.cpp | 10 | SampleRepository / OrderRepository CRUD + 영속성 |

---

## 갭 분석

| 미커버 영역 | 위치 | 중요도 |
|-------------|------|--------|
| Sample 모델 직렬화 | model/Sample.cpp | High — Order와 동일 패턴인데 누락 |
| findByName 검색 | SampleRepository.cpp | High — 한글 키워드·대소문자 미검증 |
| countByDate | OrderRepository.cpp | Medium — ID 생성 로직의 근거 |
| registerSample 유효성 검사 | SampleController.cpp:24–27 | High — yieldRate/stock 범위 분기가 핵심 |
| 재고 상태 판단 | OrderController.cpp:buildStockInfoList | High — 고갈/부족/여유 판단이 핵심 비즈니스 규칙 |
| 주문 승인 분기 | OrderController.cpp:processApproval | High — 재고 충분/부족 두 경로 모두 미검증 |
| 출고 처리 흐름 | OrderController.cpp:processRelease | Medium |

---

## 구현 전 준비 사항 (코드 변경 필요)

### 준비 1 — buildStockInfoList를 자유 함수로 승격

현재 `OrderController`의 `private` 메서드여서 직접 테스트 불가.
`BusinessLogic` 네임스페이스에 자유 함수로 이전하면 Controller도 그대로 사용하고 테스트도 직접 가능해진다.

```cpp
// util/BusinessLogic.h 추가
namespace BusinessLogic {
    std::vector<StockInfo> buildStockInfoList(
        const std::vector<Sample>& samples,
        const std::vector<Order>&  orders);
}
```

OrderController.cpp의 `buildStockInfoList` 구현을 BusinessLogic.cpp로 이동,
`OrderController::buildStockInfoList()`는 위임 호출로 변경.

### 준비 2 — View 메서드에 `virtual` 추가

Controller는 View를 참조로 받으므로, `virtual` 선언만 추가하면 gmock으로 파생 클래스 Mock 가능.
대상: `SampleView`, `OrderView`, `MonitorView`의 모든 `public` 메서드.

```cpp
// SampleView.h
virtual int         showSubMenu();
virtual SampleInput readSampleInput();
virtual bool        confirmSampleInput(const SampleInput& input);
// ...
```

소멸자도 `virtual ~SampleView() = default;` 추가.

### 준비 3 — Mock 헤더 파일 3개 작성

```
SampleOrderSystem.Tests/
├── mock/
│   ├── MockSampleView.h
│   ├── MockOrderView.h
│   └── MockMonitorView.h
```

각 Mock은 gmock `MOCK_METHOD` 매크로로 작성.

```cpp
// MockSampleView.h 예시
#include <gmock/gmock.h>
#include "view/SampleView.h"

class MockSampleView : public SampleView {
public:
    MOCK_METHOD(int,         showSubMenu,          (),                           (override));
    MOCK_METHOD(SampleInput, readSampleInput,      (),                           (override));
    MOCK_METHOD(bool,        confirmSampleInput,   (const SampleInput&),         (override));
    MOCK_METHOD(char,        showSampleList,       (const std::vector<Sample>&,
                                                    int, int, int),              (override));
    MOCK_METHOD(std::string, readSearchKeyword,    (),                           (override));
    MOCK_METHOD(void,        showSearchResult,     (const std::vector<Sample>&), (override));
    MOCK_METHOD(void,        showRegistered,       (const std::string&),         (override));
    MOCK_METHOD(void,        showNotFound,         (const std::string&),         (override));
    MOCK_METHOD(void,        showError,            (const std::string&),         (override));
};
```

---

## 테스트 파일 계획 (4개, +23개 테스트)

### test_sample_model.cpp (5개)

Phase 1에서 Order 모델은 테스트했지만 Sample 모델은 누락됐다.

| 테스트명 | 검증 내용 |
|----------|-----------|
| RoundTrip_AllFields | fromJson(toJson(s)) 후 모든 필드 일치 |
| ToJson_FieldNamesCorrect | key가 "id", "name", "avgProductionTime", "yieldRate", "stock" |
| FromJson_StockZero | stock=0 정상 역직렬화 |
| FromJson_MissingFieldThrows | 필수 필드 누락 시 예외 발생 |
| FromJson_YieldRateDecimal | yieldRate=0.92 소수점 정밀도 보존 |

### test_repository_ext.cpp (4개)

기존 test_repository.cpp에 없는 검색·집계 기능 검증.

| 테스트명 | 검증 내용 |
|----------|-----------|
| FindByName_KoreanKeyword | "웨이퍼" 검색 → 해당 시료만 반환 |
| FindByName_PartialMatch | 부분 문자열 매칭 |
| FindByName_NoMatch | 없는 키워드 → 빈 벡터 |
| CountByDate_AccumulatesCorrectly | 같은 날짜 주문 3건 추가 후 count=3 |

### test_sample_controller.cpp (6개)

Mock SampleView를 주입하여 Controller 유효성 검사 분기를 검증.
View 호출 시나리오를 `ON_CALL`로 설정하고 Repository 상태 변화로 결과 확인.

| 테스트명 | 검증 내용 |
|----------|-----------|
| RegisterSample_ValidInput_AddedToRepo | 유효 입력 → repo에 저장됨 |
| RegisterSample_InvalidYieldRate_NotAdded | yieldRate=1.5 → repo에 저장 안 됨 |
| RegisterSample_EmptyId_NotAdded | id="" → repo에 저장 안 됨 |
| RegisterSample_NegativeStock_NotAdded | stock=-1 → repo에 저장 안 됨 |
| RegisterSample_DuplicateId_ShowsError | 중복 ID → showError 1회 호출 |
| ListSamples_EmptyRepo_ShowsNotFound | 빈 repo → showNotFound 1회 호출 |

```cpp
// 테스트 픽스처 예시
class SampleControllerTest : public ::testing::Test {
protected:
    const std::string path = "test_tmp_samples_ctrl.json";
    void SetUp()    override { std::filesystem::remove(path); }
    void TearDown() override { std::filesystem::remove(path); }

    SampleRepository   repo{path};
    MockSampleView     mockView;
    SampleController   ctrl{repo, mockView};

    SampleInput makeInput(const std::string& id = "S-T01",
                          double yield = 0.9, int stock = 100) {
        return {id, "테스트시료", 0.5, yield, stock};
    }
};
```

### test_order_controller.cpp (8개)

재고 상태 판단(BusinessLogic 이전 후 직접 테스트) + 승인·출고 분기 검증.

**재고 상태 판단 — BusinessLogic::buildStockInfoList (4개)**

| 테스트명 | 검증 내용 |
|----------|-----------|
| StockStatus_Exhausted | stock=0 → "고갈" |
| StockStatus_Insufficient | stock=5, CONFIRMED 합산=10 → "부족" |
| StockStatus_Sufficient | stock=20, CONFIRMED 합산=10 → "여유" |
| StockStatus_ProducingReservedShown | PRODUCING 예약분 reservedStock 필드 정확히 계산 |

**주문 승인·출고 분기 (4개)**

| 테스트명 | 검증 내용 |
|----------|-----------|
| Approval_SufficientStock_BecomesConfirmed | 재고 충분 → 주문 CONFIRMED, stock 차감 |
| Approval_InsufficientStock_BecomesProducing | 재고 부족 → 주문 PRODUCING, 생산큐 등록 |
| Approval_Rejected_BecomesRejected | 거절 → 주문 REJECTED |
| Release_ConfirmedOrder_BecomesReleased | CONFIRMED → RELEASED |

```cpp
// 픽스처 예시 (Controller 직접 호출 대신 Repository 상태로 검증)
class OrderControllerTest : public ::testing::Test {
protected:
    SampleRepository    sRepo{"test_tmp_s.json"};
    OrderRepository     oRepo{"test_tmp_o.json"};
    ProductionLineService svc;
    MockOrderView       mockOrderView;
    MockMonitorView     mockMonitorView;
    // ProductionLineView는 이 테스트에서 호출 안 됨 → NiceMock 사용
    ::testing::NiceMock<MockProductionLineView> mockProdView;

    OrderController ctrl{sRepo, oRepo, svc,
                         mockOrderView, mockMonitorView, mockProdView};
};
```

---

## 예상 테스트 수

| Phase | 파일 수 | 테스트 수 |
|-------|---------|----------|
| Phase 1 (기존) | 5 | 35 |
| Phase 2 (추가) | 4 | +23 |
| **합계** | **9** | **58** |

---

## 구현 우선순위

| 순서 | 작업 | 이유 |
|------|------|------|
| 1 | `buildStockInfoList` → BusinessLogic 이전 | 핵심 비즈니스 규칙 직접 테스트 가능해짐 |
| 2 | test_sample_model.cpp | 코드 변경 없이 즉시 작성 가능, Phase 1 명백한 누락 |
| 3 | test_repository_ext.cpp | 코드 변경 없이 즉시 작성 가능 |
| 4 | View에 `virtual` 추가 + Mock 헤더 작성 | 다음 두 파일의 전제조건 |
| 5 | test_sample_controller.cpp | 유효성 검사 로직 |
| 6 | test_order_controller.cpp | 승인·출고 분기 (가장 복잡) |

---

## 체크리스트

- [ ] `BusinessLogic::buildStockInfoList` 자유 함수로 이전
- [ ] SampleView / OrderView / MonitorView public 메서드에 `virtual` 추가 + `virtual ~XxxView() = default`
- [ ] `SampleOrderSystem.Tests/mock/MockSampleView.h` 작성
- [ ] `SampleOrderSystem.Tests/mock/MockOrderView.h` 작성
- [ ] `SampleOrderSystem.Tests/mock/MockMonitorView.h` 작성
- [ ] test_sample_model.cpp (5개)
- [ ] test_repository_ext.cpp (4개)
- [ ] test_sample_controller.cpp (6개)
- [ ] test_order_controller.cpp (8개)
- [ ] .vcxproj에 신규 .cpp 4개 추가
