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
    // 잔여율: 전체 최대 재고 대비 현재 재고 비율 (상대적 막대 표현)
    int maxStock = 0;
    for (const auto& stockInfo : stocks)
        maxStock = std::max(maxStock, stockInfo.sample.stock);
    if (maxStock == 0) maxStock = 1;

    std::cout << "\n " << Color::BOLD << "재고 현황\n\n" << Color::RESET;
    std::cout << Color::BLUE << Color::BOLD << " "
              << padRight("시료명", 26)
              << padRight("재고", 10)
              << padRight("상태", 8)
              << "잔여율"
              << Color::RESET << "\n";
    ConsoleUI::printThinLine();

    for (const auto& stockInfo : stocks) {
        const auto& sample       = stockInfo.sample;
        int         stockPercent = sample.stock * 100 / maxStock;
        std::string stockStr     = std::to_string(sample.stock) + " ea";

        std::string reserveNote;
        if (stockInfo.reservedStock > 0) {
            reserveNote = std::string(Color::ORANGE)
                        + "(주문예약: " + std::to_string(stockInfo.reservedStock) + " ea)"
                        + Color::RESET;
        }

        std::cout << " " << padRight(sample.name, 26)
                  << padRight(stockStr, 10);
        if (!reserveNote.empty()) std::cout << reserveNote << "  ";
        std::cout << ConsoleUI::stockBadge(stockInfo.status) << "  ";
        std::cout << ConsoleUI::progressBar(stockPercent) << "\n";
    }
}

int MonitorView::showSubMenu() {
    ConsoleUI::printLine();
    std::cout << Color::BLUE << Color::BOLD << " [4] 모니터링   "
              << Color::RESET << Color::GRAY << nowString() << Color::RESET << "\n";
    ConsoleUI::printThinLine();
    std::cout << " [1] 주문량 확인    [2] 재고량 확인    [0] 뒤로\n";
    ConsoleUI::prompt("선택");
    int menuChoice = -1;
    std::cin >> menuChoice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return menuChoice;
}

void MonitorView::showOrderStats(const std::vector<Order>& orders,
                                 const std::vector<StockInfo>& stocks) {
    int reservedCount = 0, producingCount = 0, confirmedCount = 0, releasedCount = 0;
    for (const auto& order : orders) {
        switch (order.status) {
            case OrderStatus::RESERVED:  ++reservedCount;  break;
            case OrderStatus::PRODUCING: ++producingCount; break;
            case OrderStatus::CONFIRMED: ++confirmedCount; break;
            case OrderStatus::RELEASED:  ++releasedCount;  break;
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
    printRow("RESERVED",  reservedCount);
    printRow("CONFIRMED", confirmedCount);
    printRow("PRODUCING", producingCount, "\xe2\x86\x90 생산라인 대기"); // ←
    printRow("RELEASED",  releasedCount);

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
