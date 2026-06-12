#include "MonitorView.h"
#include "../util/ConsoleUI.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <limits>
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

static std::string nowString() {
    time_t now = time(nullptr);
    tm t{};
    localtime_s(&t, &now);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &t);
    return buf;
}

static void printStockTable(const std::vector<StockInfo>& stocks) {
    // 잔여율: 전체 최대 재고 대비 현재 재고 비율
    int maxStock = 0;
    for (const auto& info : stocks)
        maxStock = std::max(maxStock, info.sample.stock);
    if (maxStock == 0) maxStock = 1;

    std::cout << "\n " << Color::BOLD << "재고 현황\n\n" << Color::RESET;
    std::cout << Color::BLUE << Color::BOLD << " "
              << padRight("시료명", 26)
              << padRight("재고", 10)
              << padRight("상태", 8)
              << "잔여율"
              << Color::RESET << "\n";
    ConsoleUI::printThinLine();

    for (const auto& info : stocks) {
        const auto& s = info.sample;
        int pct = s.stock * 100 / maxStock;
        std::string stockStr = std::to_string(s.stock) + " ea";

        std::cout << " " << padRight(s.name, 26)
                  << padRight(stockStr, 10);
        std::cout << ConsoleUI::stockBadge(info.status) << "  ";
        std::cout << ConsoleUI::progressBar(pct) << "\n";
    }
}

int MonitorView::showSubMenu() {
    ConsoleUI::printLine();
    std::cout << Color::BLUE << Color::BOLD << " [4] 모니터링   "
              << Color::RESET << Color::GRAY << nowString() << Color::RESET << "\n";
    ConsoleUI::printThinLine();
    std::cout << " [1] 주문량 확인    [2] 재고량 확인    [0] 뒤로\n";
    ConsoleUI::prompt("선택");
    int c = -1;
    std::cin >> c;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return c;
}

void MonitorView::showOrderStats(const std::vector<Order>& orders,
                                 const std::vector<StockInfo>& stocks) {
    int reserved = 0, producing = 0, confirmed = 0, released = 0;
    for (const auto& o : orders) {
        switch (o.status) {
            case OrderStatus::RESERVED:  ++reserved;  break;
            case OrderStatus::PRODUCING: ++producing; break;
            case OrderStatus::CONFIRMED: ++confirmed; break;
            case OrderStatus::RELEASED:  ++released;  break;
            default: break;
        }
    }

    ConsoleUI::printThinLine();
    std::cout << "\n " << Color::BOLD << "상태별 주문 현황\n\n" << Color::RESET;

    auto printRow = [](const std::string& status, int count, const std::string& note = "") {
        std::cout << " " << ConsoleUI::statusBadge(status)
                  << "  " << std::setw(3) << count << "건";
        if (!note.empty())
            std::cout << "   " << Color::GRAY << note << Color::RESET;
        std::cout << "\n";
    };
    printRow("RESERVED",  reserved);
    printRow("CONFIRMED", confirmed);
    printRow("PRODUCING", producing, "\xe2\x86\x90 생산라인 대기"); // ←
    printRow("RELEASED",  released);

    // 재고 현황도 함께 표시
    ConsoleUI::printThinLine();
    printStockTable(stocks);
    ConsoleUI::printThinLine();
    ConsoleUI::pause();
}

void MonitorView::showStockStats(const std::vector<StockInfo>& stocks) {
    ConsoleUI::printThinLine();
    printStockTable(stocks);
    ConsoleUI::printThinLine();
    ConsoleUI::pause();
}
