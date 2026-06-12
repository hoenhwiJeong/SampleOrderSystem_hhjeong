#include "MonitorView.h"
#include "../util/ConsoleUI.h"
#include <iostream>
#include <iomanip>
#include <limits>

int MonitorView::showSubMenu() {
    ConsoleUI::printHeader("모니터링");
    std::cout << "  [1] 주문 현황 통계\n";
    std::cout << "  [2] 시료별 재고 현황\n";
    std::cout << "  [3] 생산라인 대기 현황\n";
    std::cout << "  [0] 뒤로\n";
    ConsoleUI::printThinLine();
    ConsoleUI::prompt("선택");
    int c = -1;
    std::cin >> c;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return c;
}

void MonitorView::showOrderStats(const std::vector<Order>& orders) {
    int reserved = 0, producing = 0, confirmed = 0, released = 0, rejected = 0;
    for (const auto& o : orders) {
        switch (o.status) {
            case OrderStatus::RESERVED:  ++reserved;  break;
            case OrderStatus::PRODUCING: ++producing; break;
            case OrderStatus::CONFIRMED: ++confirmed; break;
            case OrderStatus::RELEASED:  ++released;  break;
            case OrderStatus::REJECTED:  ++rejected;  break;
        }
    }

    ConsoleUI::printHeader("주문 현황 통계  총 " + std::to_string(orders.size()) + " 건");

    auto row = [](const std::string& badge, int count) {
        std::cout << "  " << badge << "  "
                  << std::setw(4) << count << " 건\n";
    };

    row(ConsoleUI::statusBadge("RESERVED"),  reserved);
    row(ConsoleUI::statusBadge("PRODUCING"), producing);
    row(ConsoleUI::statusBadge("CONFIRMED"), confirmed);
    row(ConsoleUI::statusBadge("RELEASED"),  released);
    row(ConsoleUI::statusBadge("REJECTED"),  rejected);

    ConsoleUI::printThinLine();
    int active = reserved + producing + confirmed;
    std::cout << "  활성 주문: " << active << " 건 (RESERVED + PRODUCING + CONFIRMED)\n";

    ConsoleUI::pause();
}

void MonitorView::showStockStats(const std::vector<StockInfo>& stocks) {
    ConsoleUI::printHeader("시료별 재고 현황  " + std::to_string(stocks.size()) + " 종");

    std::cout << "  " << std::left
              << std::setw(8)  << "ID"
              << std::setw(24) << "시료명"
              << std::setw(10) << "재고"
              << std::setw(10) << "CONFIRMED"
              << "상태\n";
    ConsoleUI::printThinLine();

    for (const auto& info : stocks) {
        const auto& s = info.sample;
        std::cout << "  " << std::left
                  << std::setw(8)  << s.id
                  << std::setw(24) << s.name
                  << std::setw(10) << (std::to_string(s.stock) + " ea")
                  << std::setw(10) << (std::to_string(info.confirmedTotal) + " ea")
                  << ConsoleUI::stockBadge(info.status) << "\n";
    }

    ConsoleUI::pause();
}

void MonitorView::showProducingQueue(const std::vector<Order>& producing,
                                     const std::vector<Sample>& samples) {
    ConsoleUI::printHeader("생산라인 대기 현황  " + std::to_string(producing.size()) + " 건");

    if (producing.empty()) {
        ConsoleUI::printInfo("생산 중인 주문이 없습니다.");
        ConsoleUI::pause();
        return;
    }

    std::cout << "  " << std::left
              << std::setw(22) << "주문 ID"
              << std::setw(20) << "시료명"
              << std::setw(12) << "고객사"
              << "수량\n";
    ConsoleUI::printThinLine();

    for (const auto& o : producing) {
        std::string sampleName = "?";
        for (const auto& s : samples)
            if (s.id == o.sampleId) { sampleName = s.name; break; }

        std::cout << "  " << std::left
                  << std::setw(22) << o.id
                  << std::setw(20) << sampleName
                  << std::setw(12) << o.customerName
                  << o.quantity << " ea\n";
    }

    ConsoleUI::pause();
}
