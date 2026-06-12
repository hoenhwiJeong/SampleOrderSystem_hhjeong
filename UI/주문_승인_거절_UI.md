# UI 명세 — [3] 주문 승인/거절

## 화면 구조

```
══════════════════════════════════════════════════════════════
 [3] 주문 승인/거절
──────────────────────────────────────────────────────────────
 승인 대기 중인 예약 목록  (RESERVED)

 번호    주문번호        고객              시료                   수량       상태
 [1]     ORD-0041       LG이노텍          산화막 웨이퍼-SiO2      300 ea     [RESERVED]
 [2]     ORD-0042       SK하이닉스        실리콘 웨이퍼-8인치      150 ea     [RESERVED]
 [3]     ORD-0043       삼성전자 파운드리  SiC 파워기판-6인치       200 ea     [RESERVED]
──────────────────────────────────────────────────────────────
 승인할 번호 > 3
──────────────────────────────────────────────────────────────
 재고 확인 중...

 시료          SiC 파워기판-6인치    현재 재고  30 ea
 주문 수량     200 ea                부족분     170 ea  ← 이 수량만 생산
──────────────────────────────────────────────────────────────
 재고 부족.  부족분 170 ea 승인하시겠습니까?  (실생산량 206 ea / 165 min)

 [Y] 승인    [N] 주문 거절
 선택 > Y
──────────────────────────────────────────────────────────────
 승인 완료.

 상태 변경    RESERVED  →  [PRODUCING]
 주문번호     ORD-20260416-0043
```

---

## 색상 / 스타일

| 요소 | 색상 | ANSI 코드 |
|------|------|-----------|
| 섹션 헤더 | Blue Bold | `\x1b[1;38;2;108;182;255m` |
| 테이블 헤더 행 | Blue Bold | `\x1b[1;38;2;108;182;255m` |
| `[1]`, `[2]`, `[3]` 번호 | Blue | `\x1b[38;2;108;182;255m` |
| 주문번호 | Blue | `\x1b[38;2;108;182;255m` |
| `RESERVED` 뱃지 | Blue 스타일 | `ConsoleUI::statusBadge("RESERVED")` |
| `재고 확인 중...` | Gray | `\x1b[38;2;100;100;100m` |
| 현재 재고 수치 (30 ea) | Orange | `\x1b[38;2;245;158;11m` |
| 부족분 수치 (170 ea) | Orange | `\x1b[38;2;245;158;11m` |
| 재고 부족 안내 문구 | Orange | `\x1b[38;2;245;158;11m` |
| `[Y] 승인` | Green | `\x1b[38;2;34;197;94m` |
| `[N] 주문 거절` | Red | `\x1b[38;2;239;68;68m` |
| `승인 완료.` | Green | `\x1b[38;2;34;197;94m` |
| `PRODUCING` 뱃지 | Orange 스타일 | `ConsoleUI::statusBadge("PRODUCING")` |

---

## 레이아웃 상세

### 예약 목록 테이블
```
컬럼 너비 (setw):
번호     : 8   → "[1]", "[2]" 형식
주문번호 : 14
고객     : 18
시료     : 22
수량     : 10
상태     : (뱃지)
```

### 재고 확인 섹션
- `재고 확인 중...` — Gray, 잠깐 표시되는 느낌 (실제는 즉시 출력)
- 2줄 구성:
  - `시료` + 시료명(White) + `현재 재고` + **수치(Orange)**
  - `주문 수량` + 수량 + `부족분` + **수치(Orange)** + `← 이 수량만 생산`

### 결정 문구 (Orange)
```
 재고 부족.  부족분 N ea 승인하시겠습니까?  (실생산량 N ea / N min)
```
- 전체 줄 Orange 컬러

### 재고 충분 시 분기
```
 재고 충분.  즉시 출고 대기로 전환됩니다.

 [Y] 승인    [N] 주문 거절
```
- `재고 충분.` → Green 컬러

### 승인 결과
```
 상태 변경    RESERVED  →  [PRODUCING]   (재고 부족 시)
 상태 변경    RESERVED  →  [CONFIRMED]   (재고 충분 시)
 상태 변경    RESERVED  →  [REJECTED]    (거절 시)
```

---

## ConsoleUI 구현 포인트

```cpp
// 테이블 헤더
std::cout << Color::BLUE << Color::BOLD
          << " " << std::left
          << std::setw(8)  << "번호"
          << std::setw(14) << "주문번호"
          << std::setw(18) << "고객"
          << std::setw(22) << "시료"
          << std::setw(10) << "수량"
          << "상태"
          << Color::RESET << "\n";

// 데이터 행
std::cout << " " << Color::BLUE << std::setw(8) << ("[" + std::to_string(i+1) + "]")
          << Color::BLUE << std::setw(14) << o.id
          << Color::RESET
          << std::setw(18) << o.customerName
          << std::setw(22) << sampleName
          << std::setw(10) << (std::to_string(o.quantity) + " ea")
          << ConsoleUI::statusBadge("RESERVED") << "\n";

// 재고 확인 출력 (부족 시)
ConsoleUI::printThinLine();
std::cout << Color::GRAY << " 재고 확인 중...\n\n" << Color::RESET;
std::cout << " 시료          " << s.name
          << "    현재 재고  " << Color::ORANGE << s.stock << " ea" << Color::RESET << "\n";
std::cout << " 주문 수량     " << o.quantity << " ea"
          << "                부족분     "
          << Color::ORANGE << shortage << " ea" << Color::RESET
          << "  ← 이 수량만 생산\n";

// 결정 문구
ConsoleUI::printThinLine();
std::cout << Color::ORANGE
          << " 재고 부족.  부족분 " << shortage << " ea 승인하시겠습니까?"
          << "  (실생산량 " << actualProd << " ea / " << (int)totalTime << " min)\n"
          << Color::RESET << "\n";
std::cout << Color::GREEN  << " [Y] 승인"   << Color::RESET << "    "
          << Color::RED    << "[N] 주문 거절" << Color::RESET << "\n";
ConsoleUI::prompt("선택");

// 결과
ConsoleUI::printThinLine();
std::cout << "\n " << Color::GREEN << "승인 완료." << Color::RESET << "\n\n";
std::cout << " 상태 변경    RESERVED  →  "
          << ConsoleUI::statusBadge("PRODUCING") << "\n";
std::cout << " 주문번호     " << o.id << "\n";
```

---

## 전체 출력 예시 (ANSI 제거 버전)

```
════════════════════════════════════════════════════════════
 [3] 주문 승인/거절
────────────────────────────────────────────────────────────
 승인 대기 중인 예약 목록  (RESERVED)

 번호    주문번호        고객              시료                   수량       상태
 [1]     ORD-0041       LG이노텍          산화막 웨이퍼-SiO2      300 ea     [RESERVED]
 [2]     ORD-0042       SK하이닉스        실리콘 웨이퍼-8인치      150 ea     [RESERVED]
 [3]     ORD-0043       삼성전자 파운드리  SiC 파워기판-6인치       200 ea     [RESERVED]
────────────────────────────────────────────────────────────
 승인할 번호 > 3
────────────────────────────────────────────────────────────
 재고 확인 중...

 시료          SiC 파워기판-6인치    현재 재고  30 ea
 주문 수량     200 ea                부족분     170 ea  ← 이 수량만 생산
────────────────────────────────────────────────────────────
 재고 부족.  부족분 170 ea 승인하시겠습니까?  (실생산량 206 ea / 165 min)

 [Y] 승인    [N] 주문 거절
 선택 > Y
────────────────────────────────────────────────────────────
 승인 완료.

 상태 변경    RESERVED  →  [PRODUCING]
 주문번호     ORD-20260416-0043
```
