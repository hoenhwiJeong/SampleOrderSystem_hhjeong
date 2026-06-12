#pragma once
#include <string>
#include <sstream>

// ANSI Color Codes (consoleUI.md 색상 테마 기반)
namespace Color {
    constexpr auto RESET   = "\x1b[0m";
    constexpr auto BOLD    = "\x1b[1m";
    // Primary - Blue (#6CB6FF)
    constexpr auto BLUE    = "\x1b[38;2;108;182;255m";
    // Success - Green (#22C55E)
    constexpr auto GREEN   = "\x1b[38;2;34;197;94m";
    // Warning - Orange (#F59E0B)
    constexpr auto ORANGE  = "\x1b[38;2;245;158;11m";
    // Danger - Red (#EF4444)
    constexpr auto RED     = "\x1b[38;2;239;68;68m";
    // Dim gray
    constexpr auto GRAY    = "\x1b[38;2;100;100;100m";
}

class ConsoleUI {
public:
    // ── 구분선 ──────────────────────────────────────────
    static void printLine();       // ============================================================
    static void printThinLine();   // ------------------------------------------------------------

    // ── 타이틀 / 헤더 ───────────────────────────────────
    static void printTitle();                        // ASCII 로고 + 시스템명
    static void printHeader(const std::string& title); // 섹션 헤더

    // ── 메시지 ──────────────────────────────────────────
    static void printSuccess(const std::string& msg); // >> msg
    static void printError  (const std::string& msg); // !! msg
    static void printInfo   (const std::string& msg); // -- msg

    // ── 입력 프롬프트 ────────────────────────────────────
    static void prompt(const std::string& label);  // "label > " 출력
    static bool confirm();                          // [Y/N] → true/false

    // ── 뱃지 ────────────────────────────────────────────
    static std::string statusBadge(const std::string& status); // "[CONFIRMED]" 컬러 포함
    static std::string stockBadge (const std::string& status); // "[여유]" 컬러 포함

    // ── 진행바 ───────────────────────────────────────────
    static std::string progressBar(int percent);  // "████████░░  80%"

    // ── 화면 제어 ────────────────────────────────────────
    static void clearScreen();
    static void pause();           // "계속하려면 Enter를 누르세요..."

    // ── 페이징 안내 ──────────────────────────────────────
    static void printPaging(int current, int total, int pageSize);
};
