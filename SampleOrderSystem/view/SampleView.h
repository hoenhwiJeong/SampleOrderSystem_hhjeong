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
    virtual ~SampleView() = default;

    virtual int         showSubMenu();
    virtual SampleInput readSampleInput();
    virtual bool        confirmSampleInput(const SampleInput& input);
    // returns 'N'=next page, '0'=back
    virtual char        showSampleList(const std::vector<Sample>& page,
                                       int pageNum, int totalPages, int totalCount);
    virtual std::string readSearchKeyword();
    virtual void        showSearchResult(const std::vector<Sample>& result);
    virtual void        showRegistered(const std::string& id);
    virtual void        showNotFound(const std::string& id);
    virtual void        showError(const std::string& msg);
};
