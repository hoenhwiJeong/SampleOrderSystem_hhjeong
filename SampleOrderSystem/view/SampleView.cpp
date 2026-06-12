#include "SampleView.h"
#include "../util/ConsoleUI.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <limits>

static constexpr int PAGE_SIZE = 10;

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

int SampleView::showSubMenu() {
    ConsoleUI::printLine();
    std::cout << Color::BLUE << Color::BOLD << " [1] 시료 관리" << Color::RESET << "\n";
    ConsoleUI::printThinLine();
    std::cout << " [1] 시료 등록   [2] 시료 목록   [3] 시료 검색   [0] 뒤로\n";
    ConsoleUI::prompt("선택");
    int c = -1;
    std::cin >> c;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return c;
}

SampleInput SampleView::readSampleInput() {
    ConsoleUI::printThinLine();
    std::cout << "\n " << Color::BOLD << "시료 등록\n\n" << Color::RESET;
    SampleInput in{};

    std::cout << " 시료 ID        "; ConsoleUI::prompt(""); std::getline(std::cin, in.id);
    std::cout << " 시료명         "; ConsoleUI::prompt(""); std::getline(std::cin, in.name);
    std::cout << " 평균 생산시간  "; ConsoleUI::prompt(""); std::cin >> in.avgProductionTime;
    std::cout << " 수율 (0.0-1.0) "; ConsoleUI::prompt(""); std::cin >> in.yieldRate;
    std::cout << " 초기 재고 (ea) "; ConsoleUI::prompt(""); std::cin >> in.stock;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    return in;
}

bool SampleView::confirmSampleInput(const SampleInput& in) {
    ConsoleUI::printThinLine();
    std::cout << "\n " << Color::BOLD << "입력 내용 확인\n" << Color::RESET;

    std::ostringstream timeStr, yieldStr;
    timeStr  << std::fixed << std::setprecision(1) << in.avgProductionTime << " min/ea";
    yieldStr << std::fixed << std::setprecision(2) << in.yieldRate;

    std::cout << " " << Color::GRAY << padRight("ID", 14)           << Color::RESET << in.id           << "\n";
    std::cout << " " << Color::GRAY << padRight("시료명", 14)        << Color::RESET << in.name         << "\n";
    std::cout << " " << Color::GRAY << padRight("평균 생산시간", 14) << Color::RESET << timeStr.str()   << "\n";
    std::cout << " " << Color::GRAY << padRight("수율", 14)          << Color::RESET << yieldStr.str()  << "\n";
    std::cout << " " << Color::GRAY << padRight("초기 재고", 14)     << Color::RESET << in.stock        << " ea\n\n";

    std::cout << " " << Color::GREEN << "[Y] 등록" << Color::RESET
              << "    " << Color::GRAY << "[N] 취소" << Color::RESET << "\n";
    ConsoleUI::prompt("선택");
    char c;
    std::cin >> c;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return (c == 'Y' || c == 'y');
}

char SampleView::showSampleList(const std::vector<Sample>& page,
                                int pageNum, int /*totalPages*/, int totalCount) {
    ConsoleUI::printThinLine();
    std::cout << "\n " << Color::BOLD << "등록 시료 목록  "
              << Color::RESET << Color::GRAY << "(총 " << totalCount << "종)" << Color::RESET << "\n\n";

    std::cout << Color::BLUE << Color::BOLD << " "
              << padRight("ID", 10)
              << padRight("시료명", 24)
              << padRight("평균 생산시간", 15)
              << padRight("수율", 8)
              << "현재 재고"
              << Color::RESET << "\n";
    ConsoleUI::printThinLine();

    for (const auto& s : page) {
        std::ostringstream timeStr, yieldStr;
        timeStr  << std::fixed << std::setprecision(2) << s.avgProductionTime << " min/ea";
        yieldStr << std::fixed << std::setprecision(2) << s.yieldRate;

        std::cout << " " << Color::BLUE
                  << padRight(s.id,   10)
                  << padRight(s.name, 24)
                  << Color::RESET
                  << padRight(timeStr.str(),  15)
                  << padRight(yieldStr.str(), 8)
                  << s.stock << " ea\n";
    }

    ConsoleUI::printThinLine();

    int startIdx  = (pageNum - 1) * PAGE_SIZE;
    int remaining = totalCount - (startIdx + static_cast<int>(page.size()));
    if (remaining > 0)
        std::cout << " " << Color::GRAY << "...외 " << remaining << "종"
                  << Color::RESET << "    [N] 다음페이지    [0] 뒤로\n";
    else
        std::cout << " [0] 뒤로\n";

    ConsoleUI::prompt("선택");
    char c = '0';
    std::cin >> c;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
}

std::string SampleView::readSearchKeyword() {
    ConsoleUI::printThinLine();
    std::cout << "\n " << Color::BOLD << "시료 검색\n\n" << Color::RESET;
    std::cout << " 검색어 (시료명) "; ConsoleUI::prompt("");
    std::string kw;
    std::getline(std::cin, kw);
    return kw;
}

void SampleView::showSearchResult(const std::vector<Sample>& result) {
    ConsoleUI::printThinLine();
    if (result.empty()) {
        std::cout << "\n";
        ConsoleUI::printInfo("검색 결과가 없습니다.");
        ConsoleUI::pause();
        return;
    }

    std::cout << "\n " << Color::BOLD << "검색 결과  "
              << Color::RESET << Color::GRAY << result.size() << "종" << Color::RESET << "\n\n";

    std::cout << Color::BLUE << Color::BOLD << " "
              << padRight("ID", 10)
              << padRight("시료명", 24)
              << padRight("평균 생산시간", 15)
              << padRight("수율", 8)
              << "현재 재고"
              << Color::RESET << "\n";
    ConsoleUI::printThinLine();

    for (const auto& s : result) {
        std::ostringstream timeStr, yieldStr;
        timeStr  << std::fixed << std::setprecision(2) << s.avgProductionTime << " min/ea";
        yieldStr << std::fixed << std::setprecision(2) << s.yieldRate;

        std::cout << " " << Color::BLUE
                  << padRight(s.id,   10)
                  << padRight(s.name, 24)
                  << Color::RESET
                  << padRight(timeStr.str(),  15)
                  << padRight(yieldStr.str(), 8)
                  << s.stock << " ea\n";
    }

    ConsoleUI::pause();
}

void SampleView::showRegistered(const std::string& id) {
    ConsoleUI::printThinLine();
    std::cout << "\n " << Color::GREEN << "시료 등록 완료." << Color::RESET << "\n\n";
    std::cout << " " << Color::GRAY << "ID    " << Color::RESET << id << "\n";
    ConsoleUI::pause();
}

void SampleView::showNotFound(const std::string& id) {
    ConsoleUI::printThinLine();
    ConsoleUI::printError("등록되지 않은 시료 ID입니다: " + id);
    ConsoleUI::pause();
}

void SampleView::showError(const std::string& msg) {
    ConsoleUI::printThinLine();
    ConsoleUI::printError(msg);
    ConsoleUI::pause();
}
