#include "SampleController.h"

SampleController::SampleController(SampleRepository& repo, SampleView& view)
    : repo_(repo), view_(view) {}

void SampleController::handleMenu() {
    while (true) {
        int choice = view_.showSubMenu();
        switch (choice) {
            case 1: registerSample(); break;
            case 2: listSamples();    break;
            case 3: searchSamples();  break;
            case 0: return;
            default: break;
        }
    }
}

void SampleController::registerSample() {
    SampleInput in = view_.readSampleInput();
    if (!view_.confirmSampleInput(in)) return;

    // 유효성 검사
    if (in.id.empty() || in.name.empty()) return;
    if (in.yieldRate <= 0.0 || in.yieldRate > 1.0) return;
    if (in.avgProductionTime <= 0.0)                return;
    if (in.stock < 0)                               return;

    Sample s;
    s.id                = in.id;
    s.name              = in.name;
    s.avgProductionTime = in.avgProductionTime;
    s.yieldRate         = in.yieldRate;
    s.stock             = in.stock;

    try {
        repo_.add(s);
        view_.showRegistered(s.id);
    } catch (const std::exception& e) {
        view_.showNotFound(e.what());
    }
}

void SampleController::listSamples() {
    constexpr int PAGE_SIZE = 10;
    auto all = repo_.findAll();
    if (all.empty()) {
        // showNotFound 를 범용 메시지로 활용
        view_.showNotFound("등록된 시료가 없습니다.");
        return;
    }

    int total      = static_cast<int>(all.size());
    int totalPages = (total + PAGE_SIZE - 1) / PAGE_SIZE;
    int page       = 0;

    while (true) {
        int start = page * PAGE_SIZE;
        int end   = std::min(start + PAGE_SIZE, total);
        std::vector<Sample> pageData(all.begin() + start, all.begin() + end);

        char c = view_.showSampleList(pageData, page + 1, totalPages, total);
        if (c == 'N' || c == 'n') {
            if (page + 1 < totalPages) ++page;
        } else {
            break;
        }
    }
}

void SampleController::searchSamples() {
    std::string kw = view_.readSearchKeyword();
    if (kw.empty()) return;

    auto result = repo_.findByName(kw);
    view_.showSearchResult(result);
}
