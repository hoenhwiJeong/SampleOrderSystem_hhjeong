# Phase 1 — 프로젝트 기반 세팅

## 목표

빌드 가능한 빈 프로젝트 골격 완성. 이후 모든 Phase의 기반이 된다.

## 상태: ✅ 완료

---

## 작업 목록

### 1-1. 디렉토리 구조 생성

```
SampleOrderSystem/SampleOrderSystem/
├── model/
├── repository/
├── controller/
├── service/
├── view/
├── util/
├── nlohmann/
└── data/
```

- [ ] 위 8개 폴더 생성

---

### 1-2. nlohmann/json.hpp 추가

- [ ] `nlohmann/json.hpp` (v3.11.3 단일 헤더) 복사
  - 참조: PoC2(DataPersistence) 프로젝트의 동일 파일 활용

---

### 1-3. util/JsonHelper.h/.cpp 구현

**JsonHelper.h**
```cpp
#pragma once
#include "../nlohmann/json.hpp"
#include <string>

class JsonHelper {
public:
    static nlohmann::json Load(const std::string& path);
    static void Save(const std::string& path, const nlohmann::json& j);
};
```

**JsonHelper.cpp 구현 조건**
- [ ] `Load`: 파일 없으면 빈 배열(`[]`) 반환
- [ ] `Save`: 파일을 UTF-8로 저장, 들여쓰기 4칸
- [ ] 파일 열기 실패 시 `std::runtime_error` throw

---

### 1-4. data/ 초기 파일 생성

- [ ] `data/samples.json` → 내용: `[]`
- [ ] `data/orders.json`  → 내용: `[]`

---

### 1-5. SampleOrderSystem.vcxproj 업데이트

- [ ] C++20 표준 (`stdcpp20`) 설정 확인
- [ ] x64 Debug/Release 구성 확인
- [ ] `util/JsonHelper.cpp` ClCompile 등록
- [ ] `main.cpp` ClCompile 등록
- [ ] 추후 추가될 소스 파일 등록 계획 확인

---

### 1-6. main.cpp 최소 뼈대 작성
- UI 느낌을 알려줄 수 있도록 사용될 모든 UI console 가능하도록 만들어 놓는다.
consoleUI.md 를 참조하여 작성한다.

```cpp
#include <iostream>
#include <Windows.h>

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::cout << "========================================\n";
    std::cout << "  S-Semi 반도체 시료 생산주문관리 시스템  \n";
    std::cout << "========================================\n";

    return 0;
}
```

- [ ] 빌드 성공 확인
- [ ] 한글 출력 정상 확인

---

## 완료 기준

| 항목 | 확인 |
|------|------|
| `msbuild` Debug x64 빌드 성공 (경고 없음) | ⬜ |
| 실행 시 한글 타이틀 정상 출력 | ⬜ |
| `JsonHelper::Load` 빈 JSON 파일 읽기 동작 | ⬜ |
| `JsonHelper::Save` 파일 쓰기 동작 | ⬜ |

---

## 다음 단계

Phase 1 완료 후 → **Phase 2 (Model + Repository)** 진행
