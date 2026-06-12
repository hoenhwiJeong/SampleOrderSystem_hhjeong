#include "OrderView.h"
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

// ── [2] 시료 주문 ─────────────────────────────────────────────────────────────

OrderInput OrderView::readOrderInput() {
    ConsoleUI::printLine();
    std::cout << Color::BLUE << Color::BOLD << " [2] 시료 주문" << Color::RESET << "\n";
    ConsoleUI::printThinLine();

    OrderInput in{};

    std::cout << " 시료 ID   "; ConsoleUI::prompt(""); std::getline(std::cin, in.sampleId);
    std::cout << " 고객명    "; ConsoleUI::prompt(""); std::getline(std::cin, in.customerName);
    std::cout << " 주문 수량 "; ConsoleUI::prompt(""); std::cin >> in.quantity;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    return in;
}

bool OrderView::confirmOrderInput(const OrderInput& in, const Sample& s) {
    ConsoleUI::printThinLine();
    std::cout << " " << Color::BOLD << "입력 내용 확인\n" << Color::RESET;
    std::cout << " " << Color::GRAY << padRight("시료", 8) << Color::RESET
              << s.name << "  (" << in.sampleId << ")\n";
    std::cout << " " << Color::GRAY << padRight("고객", 8) << Color::RESET
              << in.customerName << "\n";
    std::cout << " " << Color::GRAY << padRight("수량", 8) << Color::RESET
              << in.quantity << " ea\n\n";

    std::cout << " " << Color::GREEN << "[Y] 예약 접수" << Color::RESET
              << "    " << Color::GRAY << "[N] 취소" << Color::RESET << "\n";
    ConsoleUI::prompt("선택");
    char c;
    std::cin >> c;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return (c == 'Y' || c == 'y');
}

void OrderView::showOrderPlaced(const Order& o) {
    ConsoleUI::printThinLine();
    std::cout << "\n " << Color::GREEN << "예약 접수 완료." << Color::RESET << "\n\n";
    std::cout << " " << Color::GRAY << padRight("주문번호", 12) << Color::RESET << o.id << "\n";
    std::cout << " " << Color::GRAY << padRight("현재 상태", 12) << Color::RESET
              << ConsoleUI::statusBadge("RESERVED") << "\n\n";
    std::cout << Color::GRAY
              << " ※ 재고 확인은 [3] 승인 메뉴에서 직접 진행하세요."
              << Color::RESET << "\n";
    ConsoleUI::pause();
}

// ── [3] 주문 승인/거절 ─────────────────────────────────────────────────────────

int OrderView::showReservedList(const std::vector<Order>& orders,
                                const std::vector<Sample>& samples) {
    ConsoleUI::printLine();
    std::cout << Color::BLUE << Color::BOLD << " [3] 주문 승인/거절" << Color::RESET << "\n";
    ConsoleUI::printThinLine();
    std::cout << "\n " << Color::BOLD << "승인 대기 중인 예약 목록  "
              << Color::RESET << Color::GREEN << "(RESERVED)" << Color::RESET << "\n\n";

    // 테이블 헤더
    std::cout << Color::BLUE << Color::BOLD << " "
              << padRight("번호", 8)
              << padRight("주문번호", 14)
              << padRight("고객", 18)
              << padRight("시료", 22)
              << padRight("수량", 10)
              << "상태"
              << Color::RESET << "\n";
    ConsoleUI::printThinLine();

    for (int i = 0; i < static_cast<int>(orders.size()); ++i) {
        const auto& o = orders[i];
        std::string sampleName = "?";
        for (const auto& s : samples)
            if (s.id == o.sampleId) { sampleName = s.name; break; }

        std::string numTag = "[" + std::to_string(i + 1) + "]";
        std::cout << " " << Color::BLUE
                  << padRight(numTag, 8)
                  << padRight(o.id, 14)
                  << Color::RESET
                  << padRight(o.customerName, 18)
                  << padRight(sampleName, 22)
                  << padRight(std::to_string(o.quantity) + " ea", 10)
                  << ConsoleUI::statusBadge("RESERVED") << "\n";
    }

    ConsoleUI::printThinLine();
    std::cout << " 승인할 번호 ";
    ConsoleUI::prompt("");
    int sel = 0;
    std::cin >> sel;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return sel;
}

char OrderView::showApprovalDetail(const Sample& s, const Order& o,
                                   int shortage, int actualProd, double totalTime) {
    ConsoleUI::printThinLine();
    std::cout << Color::GRAY << " 재고 확인 중...\n\n" << Color::RESET;

    std::cout << " " << padRight("시료", 14) << s.name
              << "    현재 재고  " << Color::ORANGE << s.stock << " ea" << Color::RESET << "\n";
    std::cout << " " << padRight("주문 수량", 14) << o.quantity << " ea";

    if (shortage > 0) {
        std::cout << "                부족분     "
                  << Color::ORANGE << shortage << " ea" << Color::RESET
                  << "  \xe2\x86\x90 이 수량만 생산\n"; // ←
    } else {
        std::cout << "\n";
    }

    ConsoleUI::printThinLine();

    if (shortage > 0) {
        std::cout << Color::ORANGE
                  << " 재고 부족.  부족분 " << shortage
                  << " ea 승인하시겠습니까?  (실생산량 " << actualProd
                  << " ea / " << static_cast<int>(totalTime) << " min)\n"
                  << Color::RESET << "\n";
    } else {
        std::cout << Color::GREEN
                  << " 재고 충분.  즉시 출고 대기로 전환됩니다.\n"
                  << Color::RESET << "\n";
    }

    std::cout << " " << Color::GREEN  << "[Y] 승인"      << Color::RESET
              << "    " << Color::RED << "[N] 주문 거절" << Color::RESET << "\n";
    ConsoleUI::prompt("선택");
    char c;
    std::cin >> c;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (c == 'Y' || c == 'y') return 'Y';
    if (c == 'N' || c == 'n') return 'R'; // N → reject
    return '0'; // 그 외 → cancel
}

void OrderView::showApprovalResult(const Order& o) {
    ConsoleUI::printThinLine();
    if (o.status == OrderStatus::CONFIRMED || o.status == OrderStatus::PRODUCING) {
        std::cout << "\n " << Color::GREEN << "승인 완료." << Color::RESET << "\n\n";
        std::string toStatus = (o.status == OrderStatus::PRODUCING) ? "PRODUCING" : "CONFIRMED";
        std::cout << " " << Color::GRAY << padRight("상태 변경", 12) << Color::RESET
                  << "RESERVED  \xe2\x86\x92  " // →
                  << ConsoleUI::statusBadge(toStatus) << "\n";
        std::cout << " " << Color::GRAY << padRight("주문번호", 12) << Color::RESET
                  << o.id << "\n";
    } else if (o.status == OrderStatus::REJECTED) {
        std::cout << "\n " << Color::RED << "주문 거절 처리." << Color::RESET << "\n\n";
        std::cout << " " << Color::GRAY << padRight("상태 변경", 12) << Color::RESET
                  << "RESERVED  \xe2\x86\x92  "
                  << ConsoleUI::statusBadge("REJECTED") << "\n";
        std::cout << " " << Color::GRAY << padRight("주문번호", 12) << Color::RESET
                  << o.id << "\n";
    }
    ConsoleUI::pause();
}

// ── [6] 출고 처리 ─────────────────────────────────────────────────────────────

int OrderView::showConfirmedList(const std::vector<Order>& orders,
                                 const std::vector<Sample>& samples) {
    ConsoleUI::printLine();
    std::cout << Color::BLUE << Color::BOLD << " [6] 출고 처리" << Color::RESET << "\n";
    ConsoleUI::printThinLine();
    std::cout << "\n " << Color::BOLD << "출고 가능 주문  " << Color::RESET
              << Color::GREEN << "(CONFIRMED)" << Color::RESET << "\n\n";

    // 테이블 헤더
    std::cout << Color::BLUE << Color::BOLD << " "
              << padRight("번호", 8)
              << padRight("주문번호", 14)
              << padRight("고객", 14)
              << padRight("시료", 22)
              << "수량"
              << Color::RESET << "\n";
    ConsoleUI::printThinLine();

    for (int i = 0; i < static_cast<int>(orders.size()); ++i) {
        const auto& o = orders[i];
        std::string sampleName = "?";
        for (const auto& s : samples)
            if (s.id == o.sampleId) { sampleName = s.name; break; }

        std::string numTag = "[" + std::to_string(i + 1) + "]";
        std::cout << " " << Color::BLUE
                  << padRight(numTag, 8)
                  << padRight(o.id, 14)
                  << Color::RESET
                  << padRight(o.customerName, 14)
                  << padRight(sampleName, 22)
                  << o.quantity << " ea\n";
    }

    ConsoleUI::printThinLine();
    std::cout << " 출고할 번호 ";
    ConsoleUI::prompt("");
    int sel = 0;
    std::cin >> sel;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return sel;
}

void OrderView::showReleaseResult(const Order& o) {
    ConsoleUI::printThinLine();
    std::cout << "\n " << Color::GREEN << "출고 처리 완료." << Color::RESET << "\n\n";
    std::cout << " " << Color::GRAY << padRight("주문번호", 12) << Color::RESET << o.id << "\n";
    std::cout << " " << Color::GRAY << padRight("출고수량", 12) << Color::RESET << o.quantity << " ea\n";
    std::cout << " " << Color::GRAY << padRight("처리일시", 12) << Color::RESET << nowString() << "\n";
    std::cout << " " << Color::GRAY << padRight("상태", 12)    << Color::RESET
              << "CONFIRMED  \xe2\x86\x92  " << ConsoleUI::statusBadge("RELEASED") << "\n";
    ConsoleUI::pause();
}

// ── 공통 ──────────────────────────────────────────────────────────────────────

void OrderView::showNoOrders(const std::string& msg) {
    ConsoleUI::printThinLine();
    ConsoleUI::printInfo(msg);
    ConsoleUI::pause();
}

void OrderView::showSampleNotFound(const std::string& id) {
    ConsoleUI::printThinLine();
    ConsoleUI::printError("등록되지 않은 시료 ID: " + id);
    ConsoleUI::pause();
}
