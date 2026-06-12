# UI 명세 — [6] 출고 처리

## 화면 구조

```
══════════════════════════════════════════════════════════════
 [6] 출고 처리
──────────────────────────────────────────────────────────────
 출고 가능 주문  (CONFIRMED)

 번호    주문번호        고객          시료                   수량
 [1]     ORD-0042       SK하이닉스    실리콘 웨이퍼-8인치      150 ea
 [2]     ORD-0035       DB하이텍      포토레지스트-PR7         400 ea
──────────────────────────────────────────────────────────────
 출고할 번호 > 1
──────────────────────────────────────────────────────────────
 출고 처리 완료.

 주문번호    ORD-20260416-0042
 출고수량    150 ea
 처리일시    2026-04-16 09:34:02
 상태        CONFIRMED  →  [RELEASE]
──────────────────────────────────────────────────────────────
 선택 > _
```

---

## 색상 / 스타일

| 요소 | 색상 | ANSI 코드 |
|------|------|-----------|
| 섹션 헤더 `[6] 출고 처리` | Blue Bold | `\x1b[1;38;2;108;182;255m` |
| `출고 가능 주문` 소제목 | White Bold | `\x1b[1m` |
| `(CONFIRMED)` 보조 텍스트 | Green | `\x1b[38;2;34;197;94m` |
| 테이블 헤더 | Blue Bold | `\x1b[1;38;2;108;182;255m` |
| `[1]`, `[2]` 번호 | Blue | `\x1b[38;2;108;182;255m` |
| 주문번호 | Blue | `\x1b[38;2;108;182;255m` |
| `출고 처리 완료.` | Green | `\x1b[38;2;34;197;94m` |
| 결과 레이블 | Gray | `\x1b[38;2;100;100;100m` |
| 결과 값 | White | 기본 |
| `CONFIRMED →` 상태 변경 텍스트 | White | 기본 |
| `[RELEASE]` 뱃지 | Gray/Purple | `ConsoleUI::statusBadge("RELEASED")` |

---

## 레이아웃 상세

### 테이블 헤더 (상태 컬럼 없음 — CONFIRMED 목록만이므로 생략)
```
컬럼 너비 (setw):
번호     : 8   → "[1]", "[2]" 형식
주문번호 : 14
고객     : 14
시료     : 22
수량     : (끝까지)
```

### 결과 영역 (4줄)
```
 주문번호    {id}
 출고수량    {quantity} ea
 처리일시    {YYYY-MM-DD HH:MM:SS}
 상태        CONFIRMED  →  [RELEASED]
```
- 레이블(Gray, setw(12)) + 값(White) 정렬
- 마지막 줄: `CONFIRMED  →  ` + `ConsoleUI::statusBadge("RELEASED")`

### 처리일시
```cpp
time_t now = time(nullptr);
tm t{};
localtime_s(&t, &now);
char buf[20];
strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &t);
```

---

## ConsoleUI 구현 포인트

```cpp
// 소제목
std::cout << "\n " << Color::BOLD << "출고 가능 주문  " << Color::RESET
          << Color::GREEN << "(CONFIRMED)" << Color::RESET << "\n\n";

// 테이블 헤더
std::cout << Color::BLUE << Color::BOLD
          << " " << std::left
          << std::setw(8)  << "번호"
          << std::setw(14) << "주문번호"
          << std::setw(14) << "고객"
          << std::setw(22) << "시료"
          << "수량"
          << Color::RESET << "\n";
ConsoleUI::printThinLine();

// 데이터 행
for (int i = 0; i < (int)orders.size(); ++i) {
    const auto& o = orders[i];
    std::string sampleName = getSampleName(o.sampleId);
    std::cout << " " << Color::BLUE
              << std::setw(8)  << ("[" + std::to_string(i+1) + "]")
              << std::setw(14) << o.id
              << Color::RESET
              << std::setw(14) << o.customerName
              << std::setw(22) << sampleName
              << o.quantity << " ea\n";
}

// 결과
ConsoleUI::printThinLine();
std::cout << "\n " << Color::GREEN << "출고 처리 완료." << Color::RESET << "\n\n";
std::cout << Color::GRAY << " 주문번호    " << Color::RESET << o.id << "\n";
std::cout << Color::GRAY << " 출고수량    " << Color::RESET << o.quantity << " ea\n";
std::cout << Color::GRAY << " 처리일시    " << Color::RESET << nowString() << "\n";
std::cout << Color::GRAY << " 상태        " << Color::RESET
          << "CONFIRMED  →  " << ConsoleUI::statusBadge("RELEASED") << "\n";
```

---

## 전체 출력 예시 (ANSI 제거 버전)

```
════════════════════════════════════════════════════════════
 [6] 출고 처리
────────────────────────────────────────────────────────────
 출고 가능 주문  (CONFIRMED)

 번호    주문번호        고객          시료                   수량
────────────────────────────────────────────────────────────
 [1]     ORD-0042       SK하이닉스    실리콘 웨이퍼-8인치      150 ea
 [2]     ORD-0035       DB하이텍      포토레지스트-PR7         400 ea
────────────────────────────────────────────────────────────
 출고할 번호 > 1
────────────────────────────────────────────────────────────
 출고 처리 완료.

 주문번호    ORD-20260416-0042
 출고수량    150 ea
 처리일시    2026-04-16 09:34:02
 상태        CONFIRMED  →  [RELEASED]
────────────────────────────────────────────────────────────
 선택 > _
```
