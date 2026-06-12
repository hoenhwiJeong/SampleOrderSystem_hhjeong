#pragma once
#include "../model/Sample.h"
#include <vector>
#include <string>

struct SampleInput {
    std::string id;
    std::string name;
    double      avgProductionTime;
    double      yieldRate;
    int         stock;
};

class SampleView {
public:
    int         showSubMenu();
    SampleInput readSampleInput();
    bool        confirmSampleInput(const SampleInput& input);
    // returns 'N'=next page, '0'=back
    char        showSampleList(const std::vector<Sample>& page,
                               int pageNum, int totalPages, int totalCount);
    std::string readSearchKeyword();
    void        showSearchResult(const std::vector<Sample>& result);
    void        showRegistered(const std::string& id);
    void        showNotFound(const std::string& id);
};
