#include "OrderView.h"
#include "../util/ConsoleUI.h"
#include <iostream>
#include <iomanip>
#include <limits>

OrderInput OrderView::readOrderInput() {
    ConsoleUI::printHeader("시료 주문 접수");
    OrderInput in{};

    ConsoleUI::prompt("시료 ID");
    std::getline(std::cin, in.sampleId);

    ConsoleUI::prompt("고객사명");
    std::getline(std::cin, in.customerName);

    ConsoleUI::prompt("주문 수량 (ea)");
    std::cin >> in.quantity;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    return in;
}

bool OrderView::confirmOrderInput(const OrderInput& in, const Sample& s) {
    ConsoleUI::printThinLine();
    std::cout << "  시료 ID  : " << in.sampleId     << "  (" << s.name << ")\n";
    std::cout << "  고객사   : " << in.customerName  << "\n";
    std::cout << "  주문수량 : " << in.quantity       << " ea\n";
    std::cout << "  현재재고 : " << s.stock           << " ea\n";
    return ConsoleUI::confirm();
}

void OrderView::showOrderPlaced(const Order& o) {
    ConsoleUI::printSuccess("주문 접수 완료");
    std::cout << "  주문 ID : " << o.id << "\n";
    std::cout << "  상태    : " << ConsoleUI::statusBadge("RESERVED") << "\n";
    ConsoleUI::pause();
}

int OrderView::showReservedList(const std::vector<Order>& orders,
                                const std::vector<Sample>& samples) {
    ConsoleUI::printHeader("주문 승인/거절  [RESERVED " + std::to_string(orders.size()) + " 건]");

    std::cout << "  " << std::left
              << std::setw(4)  << "No"
              << std::setw(22) << "주문 ID"
              << std::setw(20) << "시료명"
              << std::setw(12) << "고객사"
              << "수량\n";
    ConsoleUI::printThinLine();

    for (int i = 0; i < static_cast<int>(orders.size()); ++i) {
        const auto& o = orders[i];
        std::string sampleName = "?";
        for (const auto& s : samples)
            if (s.id == o.sampleId) { sampleName = s.name; break; }

        std::cout << "  " << std::left
                  << std::setw(4)  << (std::to_string(i + 1) + ".")
                  << std::setw(22) << o.id
                  << std::setw(20) << sampleName
                  << std::setw(12) << o.customerName
                  << o.quantity << " ea\n";
    }

    ConsoleUI::printThinLine();
    std::cout << "  [번호] 선택   [0] 뒤로\n";
    ConsoleUI::prompt("선택");
    int sel = 0;
    std::cin >> sel;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return sel;
}

char OrderView::showApprovalDetail(const Sample& s, const Order& o,
                                   int shortage, int actualProd, double totalTime) {
    ConsoleUI::printHeader("승인 상세 검토");

    std::cout << "  주문 ID  : " << o.id << "\n";
    std::cout << "  시료     : " << s.name << " (" << s.id << ")\n";
    std::cout << "  고객사   : " << o.customerName << "\n";
    std::cout << "  주문수량 : " << o.quantity << " ea\n";
    ConsoleUI::printThinLine();

    std::cout << "  현재 재고 : " << s.stock << " ea\n";

    if (shortage <= 0) {
        std::cout << "  재고 상태 : " << ConsoleUI::stockBadge("여유") << "\n";
        std::cout << "  → 재고 차감 후 즉시 확정 처리됩니다.\n";
    } else {
        std::cout << "  부족분    : " << shortage     << " ea\n";
        std::cout << "  실 생산량 : " << actualProd   << " ea\n";
        std::cout << "  생산시간  : " << std::fixed << std::setprecision(1)
                  << totalTime << " 분\n";
        std::cout << "  재고 상태 : " << ConsoleUI::stockBadge("부족") << "\n";
        std::cout << "  → 생산라인에 투입됩니다. (PRODUCING)\n";
    }

    ConsoleUI::printThinLine();
    std::cout << "  [Y] 승인   [R] 거절   [0] 취소\n";
    ConsoleUI::prompt("선택");
    char c;
    std::cin >> c;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (c == '0') return '0';
    if (c == 'R' || c == 'r') return 'R';
    if (c == 'Y' || c == 'y') return 'Y';
    return '0';  // 그 외 입력은 취소 처리
}

void OrderView::showApprovalResult(const Order& o) {
    if (o.status == OrderStatus::CONFIRMED)
        ConsoleUI::printSuccess("주문 확정: " + o.id + "  " + ConsoleUI::statusBadge("CONFIRMED"));
    else if (o.status == OrderStatus::PRODUCING)
        ConsoleUI::printInfo   ("생산라인 투입: " + o.id + "  " + ConsoleUI::statusBadge("PRODUCING"));
    else if (o.status == OrderStatus::REJECTED)
        ConsoleUI::printError  ("주문 거절: " + o.id + "  " + ConsoleUI::statusBadge("REJECTED"));
    ConsoleUI::pause();
}

int OrderView::showConfirmedList(const std::vector<Order>& orders,
                                 const std::vector<Sample>& samples) {
    ConsoleUI::printHeader("출고 처리  [CONFIRMED " + std::to_string(orders.size()) + " 건]");

    std::cout << "  " << std::left
              << std::setw(4)  << "No"
              << std::setw(22) << "주문 ID"
              << std::setw(20) << "시료명"
              << std::setw(12) << "고객사"
              << "수량\n";
    ConsoleUI::printThinLine();

    for (int i = 0; i < static_cast<int>(orders.size()); ++i) {
        const auto& o = orders[i];
        std::string sampleName = "?";
        for (const auto& s : samples)
            if (s.id == o.sampleId) { sampleName = s.name; break; }

        std::cout << "  " << std::left
                  << std::setw(4)  << (std::to_string(i + 1) + ".")
                  << std::setw(22) << o.id
                  << std::setw(20) << sampleName
                  << std::setw(12) << o.customerName
                  << o.quantity << " ea\n";
    }

    ConsoleUI::printThinLine();
    std::cout << "  [번호] 선택   [0] 뒤로\n";
    ConsoleUI::prompt("선택");
    int sel = 0;
    std::cin >> sel;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return sel;
}

void OrderView::showReleaseResult(const Order& o) {
    ConsoleUI::printSuccess("출고 완료: " + o.id + "  " + ConsoleUI::statusBadge("RELEASED"));
    ConsoleUI::pause();
}

void OrderView::showNoOrders(const std::string& msg) {
    ConsoleUI::printInfo(msg);
    ConsoleUI::pause();
}

void OrderView::showSampleNotFound(const std::string& id) {
    ConsoleUI::printError("등록되지 않은 시료 ID: " + id);
    ConsoleUI::pause();
}
