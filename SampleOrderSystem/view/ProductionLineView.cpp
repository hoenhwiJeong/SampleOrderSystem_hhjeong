#include "ProductionLineView.h"
#include "../util/ConsoleUI.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <limits>
#include <queue>
#include <ctime>
#include <algorithm>

static int dispWidth(const std::string& s) {
    int w = 0;
    for (size_t i = 0; i < s.size(); ) {
        unsigned char c = (unsigned char)s[i];
        if      (c < 0x80) { w += 1; i += 1; }
        else if (c < 0xE0) { w += 1; i += 2; }
        else if (c < 0xF0) { w += 2; i += 3; }
        else               { w += 2; i += 4; }
    }
    return w;
}

static std::string padRight(const std::string& s, int width) {
    int pad = width - dispWidth(s);
    return s + (pad > 0 ? std::string(pad, ' ') : "");
}

// ─── Box drawing helpers ───────────────────────────────────────────────────
// UTF-8 sequences:
//   ┌ = \xe2\x94\x8c   ─ = \xe2\x94\x80   ┐ = \xe2\x94\x90
//   │ = \xe2\x94\x82   └ = \xe2\x94\x94   ┘ = \xe2\x94\x98

static constexpr int BOX_INNER = 57; // inner width between │ and │

static void boxLine(const std::string& content) {
    // Prints: "│  content\n"  (no right │ — acceptable in most terminals)
    std::cout << Color::BLUE << "\xe2\x94\x82" << Color::RESET
              << "  " << content << "\n";
}

static std::string estTime(double minutesAhead) {
    time_t now = time(nullptr);
    time_t est = now + static_cast<time_t>(minutesAhead * 60.0);
    tm t{};
    localtime_s(&t, &est);
    char buf[6];
    strftime(buf, sizeof(buf), "%H:%M", &t);
    return buf;
}

char ProductionLineView::show(const std::optional<ProductionTask>& current,
                               const std::queue<ProductionTask>&    waiting) {
    // ─── 헤더 ───────────────────────────────────────────────────────────────
    ConsoleUI::printLine();
    std::cout << Color::BLUE << Color::BOLD << " [5] 생산라인 조회   "
              << Color::RESET << Color::GRAY << "FIFO 방식" << Color::RESET << "\n";
    ConsoleUI::printThinLine();

    // ─── 상태 바 ─────────────────────────────────────────────────────────────
    std::cout << " 생산라인  1개  (단일 라인)    현재 상태: ";
    if (current.has_value())
        std::cout << Color::ORANGE << Color::BOLD << "[RUNNING]" << Color::RESET;
    else
        std::cout << Color::GRAY << "[IDLE]" << Color::RESET;
    std::cout << "\n";
    ConsoleUI::printThinLine();

    // ─── 현재 처리 중 ────────────────────────────────────────────────────────
    if (current.has_value()) {
        const auto& t = current.value();

        std::cout << Color::ORANGE << Color::BOLD
                  << " \xe2\x96\xb6 현재 처리 중\n" // ▶
                  << Color::RESET;

        // Box top: ┌──────────────────────────────────────────────────────────┐
        std::cout << Color::BLUE << "\xe2\x94\x8c"; // ┌
        for (int i = 0; i < BOX_INNER + 2; ++i) std::cout << "\xe2\x94\x80"; // ─
        std::cout << "\xe2\x94\x90\n" << Color::RESET; // ┐

        // 주문번호, 시료명
        {
            std::ostringstream ss;
            ss << padRight("주문번호", 12) << t.orderId
               << "    시료   " << t.sampleName;
            boxLine(ss.str());
        }

        // 주문량, 재고, 부족분
        {
            int originalStock = t.orderQuantity - t.shortage;
            std::ostringstream ss;
            ss << padRight("주문량", 12) << t.orderQuantity << " ea"
               << "    재고  " << originalStock << " ea  \xe2\x86\x92  부족  "; // →
            std::cout << Color::BLUE << "\xe2\x94\x82" << Color::RESET
                      << "  " << ss.str()
                      << Color::ORANGE << t.shortage << " ea" << Color::RESET << "\n";
        }

        // 실생산량, 수율, 총생산시간
        {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(2);
            ss << padRight("실생산량", 12) << t.actualProduction << " ea"
               << "   (수율 " << t.yieldRate
               << " / " << static_cast<int>(t.totalTime) << " min)";
            boxLine(ss.str());
        }

        // 진행바 + 완료 예정 (실제 경과 시간 기반)
        {
            time_t now = time(nullptr);
            double totalSec    = t.totalTime * 60.0;
            double elapsedSec  = (t.startTime > 0) ? static_cast<double>(now - t.startTime) : 0.0;
            double remainSec   = std::max(0.0, totalSec - elapsedSec);
            int    pct         = (totalSec > 0)
                                 ? std::min(100, static_cast<int>(elapsedSec * 100.0 / totalSec))
                                 : 0;
            std::ostringstream ss;
            ss << padRight("진행", 12)
               << ConsoleUI::progressBar(pct)
               << "    완료 예정  " << estTime(remainSec / 60.0);
            boxLine(ss.str());
        }

        // Box bottom: └──────────────────────────────────────────────────────────┘
        std::cout << Color::BLUE << "\xe2\x94\x94"; // └
        for (int i = 0; i < BOX_INNER + 2; ++i) std::cout << "\xe2\x94\x80"; // ─
        std::cout << "\xe2\x94\x98\n" << Color::RESET; // ┘
    } else {
        ConsoleUI::printInfo("현재 처리 중인 작업이 없습니다.");
    }

    // ─── 대기 중인 주문 (FIFO) ───────────────────────────────────────────────
    ConsoleUI::printThinLine();
    std::cout << Color::BLUE << Color::BOLD
              << " \xe2\x96\xb7 대기 중인 주문  (FIFO 순)\n" // ▷
              << Color::RESET;

    std::queue<ProductionTask> q = waiting;
    if (q.empty()) {
        std::cout << "\n";
        ConsoleUI::printInfo("대기 중인 주문이 없습니다.");
    } else {
        std::cout << "\n";
        // 테이블 헤더
        std::cout << Color::BLUE << Color::BOLD << " "
                  << padRight("순서", 8)
                  << padRight("주문번호", 14)
                  << padRight("시료", 22)
                  << padRight("주문량", 10)
                  << padRight("부족분", 10)
                  << padRight("실생산량", 10)
                  << "예상완료"
                  << Color::RESET << "\n";
        ConsoleUI::printThinLine();

        // 현재 작업의 실제 남은 시간(초) 계산
        double cumulativeMin = 0.0;
        if (current.has_value()) {
            time_t now      = time(nullptr);
            double totalSec = current->totalTime * 60.0;
            double elapsed  = (current->startTime > 0)
                              ? static_cast<double>(now - current->startTime)
                              : 0.0;
            cumulativeMin = std::max(0.0, totalSec - elapsed) / 60.0;
        }

        int idx = 1;
        while (!q.empty()) {
            const auto& t = q.front();
            cumulativeMin += t.totalTime;

            std::cout << " " << Color::BLUE
                      << padRight(std::to_string(idx), 8)
                      << padRight(t.orderId, 14)
                      << Color::RESET
                      << padRight(t.sampleName, 22)
                      << padRight(std::to_string(t.orderQuantity) + " ea", 10)
                      << padRight(std::to_string(t.shortage)       + " ea", 10)
                      << padRight(std::to_string(t.actualProduction) + " ea", 10)
                      << estTime(cumulativeMin) << "\n";
            ++idx;
            q.pop();
        }
    }

    // ─── 하단 주석 ───────────────────────────────────────────────────────────
    ConsoleUI::printThinLine();
    std::cout << Color::GRAY
              << " * 부족분 = 주문량 - 재고,  실생산량 = ceil(부족분 / (수율 \xc3\x97 0.9))\n" // ×
              << " * 선입선출(FIFO) 방식으로 처리됩니다.\n"
              << Color::RESET;

    // ─── 액션 버튼 ───────────────────────────────────────────────────────────
    ConsoleUI::printThinLine();
    if (current.has_value())
        std::cout << " " << Color::GREEN << "[C] 생산 완료 처리" << Color::RESET
                  << "    [0] 뒤로\n";
    else
        std::cout << " [0] 뒤로\n";

    ConsoleUI::prompt("선택");
    char c = '0';
    std::cin >> c;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
}

void ProductionLineView::showCompleteResult(const ProductionTask& task, int newStock) {
    ConsoleUI::printThinLine();
    std::cout << "\n " << Color::GREEN << "생산 완료 처리." << Color::RESET << "\n\n";
    std::cout << " " << Color::GRAY << padRight("주문번호", 12) << Color::RESET << task.orderId << "\n";
    std::cout << " " << Color::GRAY << padRight("시료명", 12)   << Color::RESET << task.sampleName << "\n";
    std::cout << " " << Color::GRAY << padRight("생산수량", 12) << Color::RESET << task.actualProduction << " ea\n";
    std::cout << " " << Color::GRAY << padRight("갱신 재고", 12)<< Color::RESET << newStock << " ea\n";
    std::cout << " " << Color::GRAY << padRight("주문 상태", 12)<< Color::RESET
              << "PRODUCING  \xe2\x86\x92  " << ConsoleUI::statusBadge("CONFIRMED") << "\n"; // →
    ConsoleUI::pause();
}

void ProductionLineView::showEmpty() {
    ConsoleUI::printLine();
    std::cout << Color::BLUE << Color::BOLD << " [5] 생산라인 조회   "
              << Color::RESET << Color::GRAY << "FIFO 방식" << Color::RESET << "\n";
    ConsoleUI::printThinLine();
    std::cout << "\n";
    ConsoleUI::printInfo("생산라인에 작업이 없습니다.");
    ConsoleUI::pause();
}
