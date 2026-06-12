#include "MainView.h"
#include "../util/ConsoleUI.h"
#include <iostream>
#include <iomanip>
#include <limits>

int MainView::showMenu(const SystemSummary& s) {
    ConsoleUI::printTitle();

    // 시스템 현황
    std::cout << "  시스템 현황  " << s.currentTime << "\n\n";
    std::cout << "  등록 시료  " << std::setw(4) << s.sampleCount
              << " 종      총 재고  "
              << std::setw(6) << s.totalStock << " ea\n";
    std::cout << "  전체 주문  " << std::setw(4) << s.totalOrders
              << " 건      생산대기 "
              << std::setw(4) << s.producingCount << " 건\n";
    std::cout << "  승인대기   " << std::setw(4) << s.reservedCount << " 건\n";
    ConsoleUI::printThinLine();

    // 메뉴
    std::cout << "\n";
    std::cout << "  [1] 시료 관리          [2] 시료 주문\n";
    std::cout << "  [3] 주문 승인/거절     [4] 모니터링\n";
    std::cout << "  [5] 생산라인 조회      [6] 출고 처리\n";
    std::cout << "  [0] 종료\n";
    ConsoleUI::printThinLine();

    ConsoleUI::prompt("메뉴 선택");
    int choice = -1;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return choice;
}
