# Phase 2 — Model + Repository 레이어

## 목표

데이터 구조 정의 및 JSON 파일 영속성 구현 완성.
View·Controller 없이도 데이터 읽기/쓰기가 완전히 동작하는 상태.

## 상태: ⬜ 대기

---

## 작업 목록

### 2-1. model/Sample.h/.cpp 구현

**Sample.h**
```cpp
#pragma once
#include "../nlohmann/json.hpp"
#include <string>

struct Sample {
    std::string id;
    std::string name;
    double      avgProductionTime; // min/ea
    double      yieldRate;         // 0.0 ~ 1.0
    int         stock;             // ea

    static Sample        fromJson(const nlohmann::json& j);
    nlohmann::json       toJson() const;
};
```

**구현 조건**
- [ ] `fromJson`: j["id"], j["name"], j["avgProductionTime"], j["yieldRate"], j["stock"] 파싱
- [ ] `toJson`: 위 5개 필드를 JSON 객체로 직렬화

---

### 2-2. model/Order.h/.cpp 구현

**Order.h**
```cpp
#pragma once
#include "../nlohmann/json.hpp"
#include <string>

enum class OrderStatus {
    RESERVED,
    PRODUCING,
    CONFIRMED,
    RELEASED,
    REJECTED
};

struct Order {
    std::string  id;            // ORD-YYYYMMDD-NNNN
    std::string  sampleId;
    std::string  customerName;
    int          quantity;      // ea
    OrderStatus  status;

    static Order           fromJson(const nlohmann::json& j);
    nlohmann::json         toJson() const;
    static std::string     statusToString(OrderStatus s);
    static OrderStatus     statusFromString(const std::string& s);
};
```

**statusToString 매핑**

| OrderStatus | 문자열 |
|-------------|--------|
| RESERVED | "RESERVED" |
| PRODUCING | "PRODUCING" |
| CONFIRMED | "CONFIRMED" |
| RELEASED | "RELEASED" |
| REJECTED | "REJECTED" |

**구현 조건**
- [ ] `fromJson`: 5개 필드 파싱, status는 `statusFromString()` 경유
- [ ] `toJson`: 5개 필드 직렬화, status는 `statusToString()` 경유
- [ ] `statusFromString`: 알 수 없는 문자열 입력 시 `std::runtime_error` throw

---

### 2-3. repository/SampleRepository.h/.cpp 구현

**SampleRepository.h**
```cpp
#pragma once
#include "../model/Sample.h"
#include <vector>
#include <optional>
#include <string>

class SampleRepository {
public:
    explicit SampleRepository(const std::string& filePath);

    void                     add(const Sample& s);
    std::vector<Sample>      findAll();
    std::optional<Sample>    findById(const std::string& id);
    std::vector<Sample>      findByName(const std::string& keyword);
    void                     update(const Sample& s);

private:
    std::string         filePath_;
    std::vector<Sample> data_;

    void load();
    void save();
};
```

**구현 조건**
- [ ] 생성자에서 `load()` 호출
- [ ] `add()`: ID 중복 시 `std::runtime_error("이미 존재하는 시료 ID입니다.")` throw
- [ ] `findByName()`: 대소문자 무관 부분 문자열 검색
- [ ] `update()`: ID 일치하는 항목 교체, 없으면 `std::runtime_error` throw
- [ ] `load()`: `JsonHelper::Load()` → `Sample::fromJson()` 변환
- [ ] `save()`: `Sample::toJson()` → `JsonHelper::Save()`
- [ ] 모든 쓰기 작업 후 `save()` 호출

---

### 2-4. repository/OrderRepository.h/.cpp 구현

**OrderRepository.h**
```cpp
#pragma once
#include "../model/Order.h"
#include <vector>
#include <optional>
#include <string>

class OrderRepository {
public:
    explicit OrderRepository(const std::string& filePath);

    void                     add(const Order& o);
    std::vector<Order>       findAll();
    std::optional<Order>     findById(const std::string& id);
    std::vector<Order>       findByStatus(OrderStatus status);
    void                     update(const Order& o);
    int                      countByDate(const std::string& yyyymmdd);

private:
    std::string       filePath_;
    std::vector<Order> data_;

    void load();
    void save();
};
```

**구현 조건**
- [ ] 생성자에서 `load()` 호출
- [ ] `add()`: ID 중복 시 `std::runtime_error` throw
- [ ] `findByStatus()`: 지정 상태인 주문만 필터링하여 반환
- [ ] `update()`: ID 일치하는 항목 교체, 없으면 `std::runtime_error` throw
- [ ] `countByDate()`: `id`가 `ORD-YYYYMMDD-` 로 시작하는 항목 수 반환 (주문 ID 순번 생성용)
- [ ] `load()` / `save()`: `JsonHelper` 경유

---

### 2-5. vcxproj 소스 파일 등록

- [ ] `model/Sample.cpp`
- [ ] `model/Order.cpp`
- [ ] `repository/SampleRepository.cpp`
- [ ] `repository/OrderRepository.cpp`

---

## 완료 기준

| 항목 | 확인 |
|------|------|
| Sample toJson → fromJson 왕복 값 일치 | ⬜ |
| Order toJson → fromJson 왕복 값 일치 (모든 status 포함) | ⬜ |
| SampleRepository add → findAll → 값 확인 | ⬜ |
| SampleRepository 중복 ID add 시 예외 발생 | ⬜ |
| SampleRepository findByName 부분 문자열 검색 동작 | ⬜ |
| OrderRepository findByStatus 필터링 동작 | ⬜ |
| OrderRepository countByDate 순번 반환 동작 | ⬜ |
| 프로그램 종료 후 재실행 시 데이터 유지 확인 | ⬜ |
| 빌드 성공 (경고 없음) | ⬜ |

---

## 다음 단계

Phase 2 완료 후 → **Phase 3 (Service + Controller)** 진행
