#include "ProductionLineView.h"
#include "../util/ConsoleUI.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <queue>

char ProductionLineView::show(const std::optional<ProductionTask>& current,
                               const std::queue<ProductionTask>&    waiting) {
    ConsoleUI::printHeader("생산라인 관리");

    // 현재 생산 중
    if (current.has_value()) {
        const auto& t = current.value();
        std::cout << "  ▶ 생산 중\n";
        ConsoleUI::printThinLine();
        std::cout << "  주문 ID    : " << t.orderId        << "\n";
        std::cout << "  시료       : " << t.sampleName     << " (" << t.sampleId << ")\n";
        std::cout << "  주문수량   : " << t.orderQuantity  << " ea\n";
        std::cout << "  부족분     : " << t.shortage       << " ea\n";
        std::cout << "  실 생산량  : " << t.actualProduction << " ea\n";
        std::cout << "  생산시간   : " << std::fixed << std::setprecision(1)
                  << t.totalTime << " 분\n";
    } else {
        ConsoleUI::printInfo("현재 생산 중인 작업이 없습니다.");
    }

    // 대기 큐
    std::queue<ProductionTask> q = waiting;
    if (!q.empty()) {
        ConsoleUI::printThinLine();
        std::cout << "  ▷ 대기 중 (" << q.size() << " 건)\n";
        int idx = 1;
        while (!q.empty()) {
            const auto& t = q.front();
            std::cout << "  " << idx++ << ". " << t.orderId
                      << "  " << t.sampleName
                      << "  " << t.orderQuantity << " ea\n";
            q.pop();
        }
    }

    ConsoleUI::printThinLine();
    if (current.has_value())
        std::cout << "  [C] 생산 완료 처리   [0] 뒤로\n";
    else
        std::cout << "  [0] 뒤로\n";

    ConsoleUI::prompt("선택");
    char c = '0';
    std::cin >> c;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
}

void ProductionLineView::showCompleteResult(const ProductionTask& task, int newStock) {
    ConsoleUI::printSuccess("생산 완료: " + task.orderId);
    std::cout << "  시료       : " << task.sampleName << "\n";
    std::cout << "  생산수량   : " << task.actualProduction << " ea\n";
    std::cout << "  갱신 재고  : " << newStock << " ea\n";
    std::cout << "  주문 상태  : " << ConsoleUI::statusBadge("CONFIRMED") << "\n";
    ConsoleUI::pause();
}

void ProductionLineView::showEmpty() {
    ConsoleUI::printInfo("생산라인에 작업이 없습니다.");
    ConsoleUI::pause();
}
