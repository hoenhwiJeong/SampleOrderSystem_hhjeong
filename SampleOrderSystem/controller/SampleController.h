#pragma once
#include "../repository/SampleRepository.h"
#include "../view/SampleView.h"

class SampleController {
public:
    SampleController(SampleRepository& repo, SampleView& view);

    void handleMenu();

private:
    SampleRepository& repo_;
    SampleView&       view_;

    void registerSample();
    void listSamples();
    void searchSamples();
};
