# CLAUDE.md — S-Semi 반도체 시료 생산주문관리 시스템

## 프로젝트 개요

가상의 반도체 회사 **S-Semi**의 반도체 시료 생산주문관리 시스템 (미션2 — 본 프로젝트).
PoC 검증(미션1)을 바탕으로 MVC 패턴 + JSON 영속성을 통합한 완성 시스템이다.

- **Repository**: `SampleOrderSystem_hhjeong`
- **GitHub URL**: https://github.com/hoenhwiJeong/SampleOrderSystem_hhjeong

---

## 개발 환경

- 언어: **C++20**
- IDE: Visual Studio 2022 (v145 toolset)
- 빌드 시스템: MSBuild (.slnx / .vcxproj)
- JSON 라이브러리: nlohmann/json v3.11.3 (단일 헤더)
- 플랫폼: Windows 10/11 (x64)
- 한글 출력: `SetConsoleOutputCP(CP_UTF8)` + `SetConsoleCP(CP_UTF8)`
- 사용 모델: **Sonnet / Effort: Medium** (Opus 사용 금지)

---

## 프로젝트 구조

```
SampleOrderSystem/
├── SampleOrderSystem.slnx
└── SampleOrderSystem/
    ├── SampleOrderSystem.vcxproj
    ├── main.cpp
    ├── model/
    │   ├── Sample.h/.cpp          # 시료 데이터 구조 + fromJson/toJson
    │   └── Order.h/.cpp           # 주문 데이터 구조 + OrderStatus enum + fromJson/toJson
    ├── repository/
    │   ├── SampleRepository.h/.cpp  # Sample CRUD (JSON 파일 영속성)
    │   └── OrderRepository.h/.cpp   # Order CRUD + 상태별 조회
    ├── controller/
    │   ├── SampleController.h/.cpp  # 시료 비즈니스 로직
    │   └── OrderController.h/.cpp   # 주문 접수·승인·출고·생산라인 로직
    ├── view/
    │   ├── MainView.h/.cpp         # 메인 메뉴 + 시스템 요약
    │   ├── SampleView.h/.cpp       # 시료 관련 입출력
    │   ├── OrderView.h/.cpp        # 주문 관련 입출력
    │   ├── MonitorView.h/.cpp      # 모니터링 대시보드
    │   └── ProductionLineView.h/.cpp # 생산라인 현황
    ├── service/
    │   └── ProductionLineService.h/.cpp  # FIFO 생산 큐 관리
    ├── util/
    │   └── JsonHelper.h/.cpp       # Load/Save 정적 유틸
    ├── nlohmann/
    │   └── json.hpp                # nlohmann/json v3.11.3
    └── data/
        ├── samples.json            # 시료 영속 데이터
        └── orders.json             # 주문 영속 데이터
```

---

## 레이어 규칙

- **View**: 출력·입력 전용. `std::cin/cout` 사용. 비즈니스 로직 없음.
- **Controller**: View + Repository 연결. 비즈니스 로직 담당.
- **Model**: 순수 데이터 구조 + `fromJson`/`toJson`. 로직 없음.
- **Repository**: JSON 파일 CRUD 캡슐화. `load()`/`save()` private.
- **Service**: 생산라인 큐 등 도메인 서비스.

---

## 데이터 모델

### Sample

| 필드 | 타입 | 설명 |
|------|------|------|
| id | string | 시료 ID (예: S-001) |
| name | string | 시료명 |
| avgProductionTime | double | 평균 생산시간 (분/ea) |
| yieldRate | double | 수율 (0.0~1.0, 예: 0.92) |
| stock | int | 현재 재고 수량 (ea) |

### Order

| 필드 | 타입 | 설명 |
|------|------|------|
| id | string | 주문 ID (ORD-YYYYMMDD-NNNN) |
| sampleId | string | 주문 시료 ID |
| customerName | string | 고객명 |
| quantity | int | 주문 수량 (ea) |
| status | OrderStatus | 주문 상태 |

### OrderStatus enum

```cpp
enum class OrderStatus {
    RESERVED,   // 주문 접수
    PRODUCING,  // 생산 중 (재고 부족)
    CONFIRMED,  // 출고 대기 (재고 충분)
    RELEASED,   // 출고 완료
    REJECTED    // 주문 거절
};
```

---

## 주문 상태 흐름

```
RESERVED ──(승인·재고 충분)──→ CONFIRMED ──(출고 처리)──→ RELEASED
         ──(승인·재고 부족)──→ PRODUCING ──(생산 완료)──→ CONFIRMED
         ──(거절)───────────→ REJECTED
```

- **REJECTED**는 모니터링에서 제외한다.

---

## 메인 메뉴 구성

| 번호 | 메뉴 | 기능 |
|------|------|------|
| 1 | 시료 관리 | 시료 등록·조회·검색 |
| 2 | 시료 주문 | 주문 접수 (→ RESERVED) |
| 3 | 주문 승인/거절 | RESERVED 목록 확인 후 승인 또는 거절 |
| 4 | 모니터링 | 상태별 주문 수·시료별 재고 현황 |
| 5 | 생산라인 조회 | 현재 생산 중 및 대기 큐 확인 |
| 6 | 출고 처리 | CONFIRMED 주문 출고 (→ RELEASED) |
| 0 | 종료 | 프로그램 종료 |

메인 메뉴 진입 시 시스템 요약(등록 시료 수, 총 재고, 전체 주문 수, 생산라인 대기)을 표시한다.

---

## 핵심 비즈니스 로직

### 주문 승인 로직

```cpp
int shortage = order.quantity - sample.stock;

if (shortage <= 0) {
    // 재고 충분 → 즉시 CONFIRMED, 재고 차감
    sample.stock -= order.quantity;
    order.status = OrderStatus::CONFIRMED;
} else {
    // 재고 부족 → 생산라인 등록, PRODUCING으로 전환
    int actualProduction = (int)std::ceil(shortage / (sample.yieldRate * 0.9));
    double totalTime = sample.avgProductionTime * actualProduction;
    order.status = OrderStatus::PRODUCING;
    productionLineService.enqueue(order, shortage, actualProduction, totalTime);
}
```

승인 시 화면에 아래 정보를 표시한다:
```
재고 부족. 부족분 170 ea 승인하겠습니까? (실생산량 206 ea / 165 min)
```

### 생산 완료 처리

```cpp
// 생산 완료 후 재고 반영: 기존재고 + 실생산량 - 주문수량
sample.stock = sample.stock + actualProduction - order.quantity;
order.status = OrderStatus::CONFIRMED;
```

예시 (SiC 파워기판, 재고 30 ea, 주문 80 ea):
- 부족분 = 80 - 30 = 50 ea
- 실 생산량 = ceil(50 / (0.92 × 0.9)) = ceil(60.4) = **61 ea**
- 총 생산시간 = 0.8 min/ea × 61 = **48.8 min**
- 생산 후 재고 = 30 + 61 - 80 = **11 ea**

### 재고 상태 판단 (모니터링)

```cpp
// CONFIRMED 주문의 해당 시료 합산 수량
int confirmedTotal = /* 해당 시료의 CONFIRMED 주문 수량 합 */;

if (sample.stock == 0)                  stockStatus = "고갈";
else if (sample.stock < confirmedTotal) stockStatus = "부족";
else                                    stockStatus = "여유";
```

### 주문 ID 형식

```
ORD-YYYYMMDD-NNNN   (예: ORD-20260612-0001)
```

---

## 생산라인 상세

### 구조

- 생산라인은 **단일 라인** 1개 (하나씩 순차 처리)
- 스케줄링: **FIFO** (선입선출, `std::queue<ProductionTask>`)
- 현재 처리 중인 작업 1건 + 대기 큐로 구성

### ProductionTask 구조

```cpp
struct ProductionTask {
    std::string orderId;        // 주문 ID
    std::string sampleId;       // 시료 ID
    std::string sampleName;     // 시료명 (표시용)
    int         orderQuantity;  // 주문 수량
    int         shortage;       // 부족분 = 주문량 - 당시 재고
    int         actualProduction; // 실 생산량 = ceil(부족분 / (수율 * 0.9))
    double      totalTime;      // 총 생산시간 = 평균생산시간 * 실생산량 (분)
};
```

### 실 생산량 공식

```
부족분        = 주문 수량 - 현재 재고
실 생산량     = ceil(부족분 / (수율 × 0.9))
총 생산 시간  = 평균 생산시간(min/ea) × 실 생산량
```

수율에 0.9를 곱하는 이유: 공정 오차(불량률 10%)를 감안한 안전 마진 확보.

### 생산라인 조회 화면 표시 항목

**현재 처리 중인 작업:**
- 주문번호, 시료명, 주문 수량
- 현재 재고 → 부족분 → 실 생산량 (수율 / 총 생산시간)
- 진행률(%) 및 완료 예정 시각

**대기 중인 주문 목록 (FIFO 순):**

| 순서 | 주문번호 | 시료 | 주문량 | 부족분 | 실생산량 | 예상완료 |
|------|----------|------|--------|--------|----------|----------|
| 1 | ORD-... | 시료명 | N ea | N ea | N ea | HH:MM |

### 생산 처리 흐름

```
주문 승인(재고 부족)
  → ProductionTask 생성 및 큐 enqueue
  → 주문 상태: RESERVED → PRODUCING

생산라인 처리 (메뉴 5 또는 자동)
  → 큐에서 dequeue
  → 생산 완료: 재고 반영 (stock + actualProduction - quantity)
  → 주문 상태: PRODUCING → CONFIRMED

다음 대기 작업이 있으면 자동으로 처리 시작
```

> **참고**: 이 시스템은 콘솔 기반이므로 실시간 타이머는 없다.
> 생산라인 조회 메뉴 진입 시 또는 별도 "생산 완료 처리" 액션으로 완료를 수동 트리거한다.

---

## 데이터 파일

- 실행 파일 기준 `data/samples.json`, `data/orders.json`
- 앱 재시작 후에도 데이터 유지 (영속성)
- 중복 ID 삽입 시 `std::runtime_error` throw

---

## 빌드 및 실행

```
SampleOrderSystem\SampleOrderSystem.slnx   ← Visual Studio 2022에서 열기
```

또는 Developer Command Prompt에서:

```
msbuild SampleOrderSystem\SampleOrderSystem.slnx /p:Configuration=Debug /p:Platform=x64
```

실행 시 작업 디렉토리는 `C:\PoCDev\SampleOrderSystem\SampleOrderSystem\` 로 설정하면
`data/` 폴더를 자동으로 참조한다.

---

## UI 스타일 가이드

모든 View는 `util/ConsoleUI.h` 를 사용하여 일관된 화면을 구성한다.
UI 디자인 컨셉 및 컴포넌트 명세는 **`consoleUI.md`** 를 반드시 참조한다.

- 디자인 컨셉: Cyberpunk / Industrial Dashboard / Dark Theme
- 색상 테마: Primary(Blue `#6CB6FF`), Success(Green `#22C55E`), Warning(Orange `#F59E0B`), Danger(Red `#EF4444`)
- 구분선: `ConsoleUI::printLine()` / `printThinLine()`
- 상태 뱃지: `ConsoleUI::statusBadge()` / `stockBadge()`
- 진행바: `ConsoleUI::progressBar(percent)`
- 메시지: `printSuccess()` / `printError()` / `printInfo()`
- ANSI Escape Code + UTF-8 + Unicode Box Drawing 문자 사용

---

## 개발 계획 (Plan)

개발은 5개 Phase로 나눠 순차 진행한다. 상세 내용은 `Plan/` 디렉토리 참조.

```
Plan/
├── plan.md     — 전체 Phase 요약 및 진행 현황
├── phase1.md   — 프로젝트 기반 세팅 (디렉토리, JsonHelper, vcxproj)
├── phase2.md   — Model + Repository (Sample/Order, CRUD)
├── phase3.md   — Service + Controller (생산라인, 비즈니스 로직)
├── phase4.md   — View + main.cpp 통합
└── phase5.md   — 통합 테스트 및 검증
```

| Phase | 내용 | 상태 |
|-------|------|------|
| 1 | 프로젝트 기반 세팅 | ✅ 완료 |
| 2 | Model + Repository | ⬜ 대기 |
| 3 | Service + Controller | ⬜ 대기 |
| 4 | View + main.cpp 통합 | ⬜ 대기 |
| 5 | 통합 테스트 및 검증 | ⬜ 대기 |

---

## 유의사항
- 커밋 이력을 남기며 개발 진행

## 참고
[과제1] 은 현재 완료상태이다.
git 주소는 각각 아래와 같고 어느정도 참조하여 프로젝트를 진행한다
1	ConsoleMVC_hhjeong		https://github.com/hoenhwiJeong/ConsoleMVC_hhjeong
2	DataPersistence_hhjeong		https://github.com/hoenhwiJeong/DataPersistence_hhjeong
3	DataMonitor_hhjeong		https://github.com/hoenhwiJeong/DataMonitor_hhjeong
4	DummyDataGenerator_hhjeong	https://github.com/hoenhwiJeong/DummyDataGenerator_hhjeong
