#pragma once
#include <string>

struct SystemSummary {
    int sampleCount;
    int totalStock;
    int totalOrders;
    int producingCount;
    int reservedCount;
    std::string currentTime;
};

class MainView {
public:
    int showMenu(const SystemSummary& s);
};
