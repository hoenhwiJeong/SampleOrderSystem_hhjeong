# UI 명세 — [4] 모니터링

## 화면 구조

```
══════════════════════════════════════════════════════════════
 [4] 모니터링   2026-04-16 09:32:15
──────────────────────────────────────────────────────────────
 [1] 주문량 확인    [2] 재고량 확인    [0] 뒤로
 선택 > 1
──────────────────────────────────────────────────────────────
 상태별 주문 현황

 [RESERVED ]   3건
 [CONFIRMED]   8건
 [PRODUCING]   3건   ← 생산라인 대기
 [RELEASE  ]  18건
──────────────────────────────────────────────────────────────
 재고 현황

 시료명                    재고        상태     잔여율
 실리콘 웨이퍼-8인치        480 ea     [여유]   ██████████  80%
 GaN 에피택셜-4인치         220 ea     [여유]   █████░░░░░  44%
 SiC 파워기판-6인치          30 ea     [부족]   █░░░░░░░░░   6%
 산화막 웨이퍼-SiO2           0 ea     [고갈]   ░░░░░░░░░░   0%
──────────────────────────────────────────────────────────────
 선택 > _
```

---

## 색상 / 스타일

| 요소 | 색상 | ANSI 코드 |
|------|------|-----------|
| 섹션 헤더 | Blue Bold | `\x1b[1;38;2;108;182;255m` |
| 날짜/시간 | Gray | `\x1b[38;2;100;100;100m` |
| `상태별 주문 현황` 소제목 | White Bold | `\x1b[1m` |
| `[RESERVED]` 뱃지 | Blue | `ConsoleUI::statusBadge("RESERVED")` |
| `[CONFIRMED]` 뱃지 | Green | `ConsoleUI::statusBadge("CONFIRMED")` |
| `[PRODUCING]` 뱃지 | Orange | `ConsoleUI::statusBadge("PRODUCING")` |
| `[RELEASE]` 뱃지 | Gray/Purple | `ConsoleUI::statusBadge("RELEASED")` |
| `← 생산라인 대기` 주석 | Gray | `\x1b[38;2;100;100;100m` |
| `재고 현황` 소제목 | White Bold | `\x1b[1m` |
| 테이블 헤더 | Blue Bold | `\x1b[1;38;2;108;182;255m` |
| `[여유]` 뱃지 | Green | `ConsoleUI::stockBadge("여유")` |
| `[부족]` 뱃지 | Orange | `ConsoleUI::stockBadge("부족")` |
| `[고갈]` 뱃지 | Red | `ConsoleUI::stockBadge("고갈")` |
| 진행바 (80% 이상) | Green | `ConsoleUI::progressBar(pct)` |
| 진행바 (20~49%) | Orange | `ConsoleUI::progressBar(pct)` |
| 진행바 (1~19%) | Orange/Red | `ConsoleUI::progressBar(pct)` |
| 진행바 (0%) | Gray 빈 블록 | `ConsoleUI::progressBar(0)` |

---

## 레이아웃 상세

### 헤더 라인
```
 [4] 모니터링   {현재 날짜/시간}
```
- 헤더(Blue Bold) + 날짜(Gray) 한 줄

### 상태별 주문 현황 섹션
- 뱃지 + 건수를 한 줄씩
- PRODUCING 줄에만 `← 생산라인 대기` 주석(Gray) 추가

### 재고 현황 테이블
```
컬럼 너비 (setw):
시료명   : 26
재고     : 10
상태 뱃지: 10 (출력 후 공백 조정)
잔여율   : progressBar + 수치
```

### 잔여율 계산
```cpp
// 최대 재고 기준치 설정 필요 (예: 각 시료의 최대 재고 대비 현재 비율)
// 또는 고정 최대값(예: 1000) 대비 %
int pct = (maxStock > 0) ? (sample.stock * 100 / maxStock) : 0;
```
> 이미지 기준: 480/600=80%, 220/500=44%, 30/500=6%, 0/500=0%
> 실제 구현 시 각 시료의 초기 최대 재고 기준 또는 별도 상수 사용 권장

---

## ConsoleUI 구현 포인트

```cpp
// 헤더 (날짜 포함)
std::cout << Color::BLUE << Color::BOLD << " [4] 모니터링   " << Color::RESET
          << Color::GRAY << nowString() << Color::RESET << "\n";

// 상태 뱃지 + 건수
auto printStatusRow = [](const std::string& status, int count, const std::string& note = "") {
    std::cout << " " << ConsoleUI::statusBadge(status)
              << "  " << std::setw(3) << count << "건";
    if (!note.empty())
        std::cout << Color::GRAY << "   " << note << Color::RESET;
    std::cout << "\n";
};
printStatusRow("RESERVED",  reserved);
printStatusRow("CONFIRMED", confirmed);
printStatusRow("PRODUCING", producing, "← 생산라인 대기");
printStatusRow("RELEASED",  released);

// 재고 현황 테이블 헤더
ConsoleUI::printThinLine();
std::cout << "\n " << Color::BOLD << "재고 현황\n\n" << Color::RESET;
std::cout << Color::BLUE << Color::BOLD
          << " " << std::left
          << std::setw(26) << "시료명"
          << std::setw(10) << "재고"
          << std::setw(10) << "상태"
          << "잔여율"
          << Color::RESET << "\n";
ConsoleUI::printThinLine();

// 재고 데이터 행
for (const auto& info : stocks) {
    int pct = (info.sample.stock > 0) ? std::min(100, info.sample.stock * 100 / MAX_STOCK) : 0;
    std::cout << " " << std::left
              << std::setw(26) << info.sample.name
              << std::setw(10) << (std::to_string(info.sample.stock) + " ea")
              << std::setw(10) << ""   // 뱃지 출력 전 공백
              << ConsoleUI::stockBadge(info.status) << "  "
              << ConsoleUI::progressBar(pct) << "\n";
}
```

---

## 전체 출력 예시 (ANSI 제거 버전)

```
════════════════════════════════════════════════════════════
 [4] 모니터링   2026-04-16 09:32:15
────────────────────────────────────────────────────────────
 [1] 주문량 확인    [2] 재고량 확인    [0] 뒤로
 선택 > 1
────────────────────────────────────────────────────────────
 상태별 주문 현황

 [RESERVED ]    3건
 [CONFIRMED]    8건
 [PRODUCING]    3건   ← 생산라인 대기
 [RELEASED ]   18건
────────────────────────────────────────────────────────────
 재고 현황

 시료명                    재고        상태     잔여율
────────────────────────────────────────────────────────────
 실리콘 웨이퍼-8인치        480 ea     [여유]   ████████░░  80%
 GaN 에피택셜-4인치         220 ea     [여유]   ████░░░░░░  44%
 SiC 파워기판-6인치          30 ea     [부족]   █░░░░░░░░░   6%
 산화막 웨이퍼-SiO2           0 ea     [고갈]   ░░░░░░░░░░   0%
────────────────────────────────────────────────────────────
 선택 > _
```
