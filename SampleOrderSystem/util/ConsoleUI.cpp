#include "ConsoleUI.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <limits>

static constexpr int LINE_WIDTH = 60;

void ConsoleUI::printLine() {
    std::cout << Color::GRAY;
    for (int i = 0; i < LINE_WIDTH; ++i)
        std::cout << "\xe2\x95\x90"; // ═ (U+2550)
    std::cout << Color::RESET << "\n";
}

void ConsoleUI::printThinLine() {
    std::cout << Color::GRAY;
    for (int i = 0; i < LINE_WIDTH; ++i)
        std::cout << "\xe2\x94\x80"; // ─ (U+2500)
    std::cout << Color::RESET << "\n";
}

void ConsoleUI::printTitle() {
    clearScreen();
    // ASCII 로고 (S-Semi / MES 스타일)
    std::cout << Color::BLUE << Color::BOLD;
    std::cout << " ███████╗       ███████╗███████╗███╗   ███╗██╗\n";
    std::cout << " ██╔════╝       ██╔════╝██╔════╝████╗ ████║██║\n";
    std::cout << " ███████╗ ───   ███████╗█████╗  ██╔████╔██║██║\n";
    std::cout << " ╚════██║       ╚════██║██╔══╝  ██║╚██╔╝██║██║\n";
    std::cout << " ███████║       ███████║███████╗██║ ╚═╝ ██║██║\n";
    std::cout << " ╚══════╝       ╚══════╝╚══════╝╚═╝     ╚═╝╚═╝\n";
    std::cout << Color::RESET;
    printLine();
    std::cout << Color::BLUE << Color::BOLD
              << "       S-Semi 반도체 시료 생산주문관리 시스템\n"
              << Color::RESET;
    printLine();
}

void ConsoleUI::printHeader(const std::string& title) {
    printThinLine();
    std::cout << " " << Color::BLUE << Color::BOLD
              << title << Color::RESET << "\n";
    printThinLine();
}

void ConsoleUI::printSuccess(const std::string& msg) {
    std::cout << " " << Color::GREEN << ">>" << Color::RESET
              << " " << msg << "\n";
}

void ConsoleUI::printError(const std::string& msg) {
    std::cout << " " << Color::RED << "!!" << Color::RESET
              << " " << msg << "\n";
}

void ConsoleUI::printInfo(const std::string& msg) {
    std::cout << " " << Color::GRAY << "--" << Color::RESET
              << " " << msg << "\n";
}

void ConsoleUI::prompt(const std::string& label) {
    std::cout << Color::BLUE << " " << label << " > " << Color::RESET;
}

bool ConsoleUI::confirm() {
    std::cout << "\n " << Color::GRAY << "[Y]" << Color::RESET << " 확인    "
              << Color::GRAY << "[N]" << Color::RESET << " 취소\n";
    prompt("선택");
    char c;
    std::cin >> c;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return (c == 'Y' || c == 'y');
}

std::string ConsoleUI::statusBadge(const std::string& status) {
    std::string color;
    std::string padded;

    if (status == "RESERVED")       { color = Color::BLUE;   padded = "RESERVED "; }
    else if (status == "PRODUCING")  { color = Color::ORANGE; padded = "PRODUCING"; }
    else if (status == "CONFIRMED")  { color = Color::GREEN;  padded = "CONFIRMED"; }
    else if (status == "RELEASED")   { color = Color::GRAY;   padded = "RELEASED "; }
    else if (status == "REJECTED")   { color = Color::RED;    padded = "REJECTED "; }
    else                             { color = Color::GRAY;   padded = status;       }

    return std::string(color) + "[" + padded + "]" + Color::RESET;
}

std::string ConsoleUI::stockBadge(const std::string& status) {
    std::string color;
    if      (status == "여유") color = Color::GREEN;
    else if (status == "부족") color = Color::ORANGE;
    else if (status == "고갈") color = Color::RED;
    else                       color = Color::GRAY;

    return std::string(color) + "[" + status + "]" + Color::RESET;
}

std::string ConsoleUI::progressBar(int percent) {
    percent = std::max(0, std::min(100, percent));
    int filled = percent / 10;

    std::string bar;
    // 채워진 블록
    if (filled > 0) {
        std::string color = (percent >= 50) ? Color::GREEN
                          : (percent >= 20) ? Color::ORANGE
                          :                   Color::RED;
        bar += color;
        for (int i = 0; i < filled; ++i) bar += "\xe2\x96\x88"; // █
        bar += Color::RESET;
    }
    // 빈 블록
    bar += Color::GRAY;
    for (int i = filled; i < 10; ++i) bar += "\xe2\x96\x91"; // ░
    bar += Color::RESET;

    std::ostringstream oss;
    oss << bar << " " << std::setw(3) << percent << "%";
    return oss.str();
}

void ConsoleUI::clearScreen() {
    // ANSI 화면 지우기 + 커서 홈
    std::cout << "\x1b[2J\x1b[H";
}

void ConsoleUI::pause() {
    std::cout << "\n " << Color::GRAY
              << "계속하려면 Enter를 누르세요..."
              << Color::RESET;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void ConsoleUI::printPaging(int current, int total, int pageSize) {
    int remaining = total - (current + 1) * pageSize;
    printThinLine();
    if (remaining > 0) {
        std::cout << " " << Color::GRAY << "...외 " << remaining << "종"
                  << Color::RESET
                  << "    [N] 다음 페이지    [0] 뒤로\n";
    } else {
        std::cout << "    [0] 뒤로\n";
    }
}
