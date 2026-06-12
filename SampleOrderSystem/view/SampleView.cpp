#include "SampleView.h"
#include "../util/ConsoleUI.h"
#include <iostream>
#include <iomanip>
#include <limits>

static constexpr int PAGE_SIZE = 10;

int SampleView::showSubMenu() {
    ConsoleUI::printHeader("시료 관리");
    std::cout << "  [1] 시료 등록\n";
    std::cout << "  [2] 시료 목록\n";
    std::cout << "  [3] 시료 검색\n";
    std::cout << "  [0] 뒤로\n";
    ConsoleUI::printThinLine();
    ConsoleUI::prompt("선택");
    int c = -1;
    std::cin >> c;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return c;
}

SampleInput SampleView::readSampleInput() {
    ConsoleUI::printHeader("시료 등록");
    SampleInput in{};

    ConsoleUI::prompt("시료 ID (예: S-006)");
    std::getline(std::cin, in.id);

    ConsoleUI::prompt("시료명");
    std::getline(std::cin, in.name);

    ConsoleUI::prompt("평균 생산시간 (분/ea)");
    std::cin >> in.avgProductionTime;

    ConsoleUI::prompt("수율 (0.0 ~ 1.0)");
    std::cin >> in.yieldRate;

    ConsoleUI::prompt("초기 재고 (ea)");
    std::cin >> in.stock;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    return in;
}

bool SampleView::confirmSampleInput(const SampleInput& in) {
    ConsoleUI::printThinLine();
    std::cout << "  ID       : " << in.id                  << "\n";
    std::cout << "  시료명   : " << in.name                 << "\n";
    std::cout << "  생산시간 : " << in.avgProductionTime    << " min/ea\n";
    std::cout << "  수율     : " << in.yieldRate * 100.0    << " %\n";
    std::cout << "  초기재고 : " << in.stock                << " ea\n";
    return ConsoleUI::confirm();
}

char SampleView::showSampleList(const std::vector<Sample>& page,
                                int pageNum, int totalPages, int totalCount) {
    ConsoleUI::printHeader("시료 목록  [" + std::to_string(pageNum) + "/"
                           + std::to_string(totalPages) + " page]  총 "
                           + std::to_string(totalCount) + " 종");

    // 헤더
    std::cout << "  " << std::left
              << std::setw(8)  << "ID"
              << std::setw(24) << "시료명"
              << std::setw(10) << "생산시간"
              << std::setw(8)  << "수율"
              << std::setw(8)  << "재고"
              << "상태\n";
    ConsoleUI::printThinLine();

    for (const auto& s : page) {
        int maxStock = 1000;
        int pct      = (s.stock > 0) ? (s.stock * 100 / maxStock) : 0;
        pct = std::min(pct, 100);
        std::string stockSt = (s.stock == 0) ? "고갈"
                            : (s.stock < 50)  ? "부족"
                            :                   "여유";

        std::cout << "  " << std::left
                  << std::setw(8)  << s.id
                  << std::setw(24) << s.name
                  << std::setw(10) << (std::to_string(s.avgProductionTime) + "m")
                  << std::setw(8)  << (std::to_string((int)(s.yieldRate * 100)) + "%")
                  << std::setw(8)  << (std::to_string(s.stock) + "ea")
                  << ConsoleUI::stockBadge(stockSt) << "\n";
    }

    ConsoleUI::printThinLine();
    if (pageNum < totalPages)
        std::cout << "  [N] 다음 페이지   [0] 뒤로\n";
    else
        std::cout << "  [0] 뒤로\n";
    ConsoleUI::prompt("선택");

    char c = '0';
    std::cin >> c;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return c;
}

std::string SampleView::readSearchKeyword() {
    ConsoleUI::printHeader("시료 검색");
    ConsoleUI::prompt("검색어 (시료명)");
    std::string kw;
    std::getline(std::cin, kw);
    return kw;
}

void SampleView::showSearchResult(const std::vector<Sample>& result) {
    if (result.empty()) {
        ConsoleUI::printInfo("검색 결과가 없습니다.");
        ConsoleUI::pause();
        return;
    }

    ConsoleUI::printHeader("검색 결과  " + std::to_string(result.size()) + " 종");
    std::cout << "  " << std::left
              << std::setw(8)  << "ID"
              << std::setw(24) << "시료명"
              << std::setw(10) << "생산시간"
              << std::setw(8)  << "수율"
              << "재고\n";
    ConsoleUI::printThinLine();

    for (const auto& s : result) {
        std::cout << "  " << std::left
                  << std::setw(8)  << s.id
                  << std::setw(24) << s.name
                  << std::setw(10) << (std::to_string(s.avgProductionTime) + "m")
                  << std::setw(8)  << (std::to_string((int)(s.yieldRate * 100)) + "%")
                  << s.stock << " ea\n";
    }

    ConsoleUI::pause();
}

void SampleView::showRegistered(const std::string& id) {
    ConsoleUI::printSuccess("시료 등록 완료: " + id);
    ConsoleUI::pause();
}

void SampleView::showNotFound(const std::string& id) {
    ConsoleUI::printError("등록되지 않은 시료 ID입니다: " + id);
    ConsoleUI::pause();
}

void SampleView::showError(const std::string& msg) {
    ConsoleUI::printError(msg);
    ConsoleUI::pause();
}
