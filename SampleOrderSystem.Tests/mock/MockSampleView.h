#pragma once
#include <gmock/gmock.h>
#include "view/SampleView.h"

class MockSampleView : public SampleView {
public:
    MOCK_METHOD(int,         showSubMenu,         (),                                             (override));
    MOCK_METHOD(SampleInput, readSampleInput,     (),                                             (override));
    MOCK_METHOD(bool,        confirmSampleInput,  (const SampleInput&),                           (override));
    MOCK_METHOD(char,        showSampleList,      (const std::vector<Sample>&, int, int, int),    (override));
    MOCK_METHOD(std::string, readSearchKeyword,   (),                                             (override));
    MOCK_METHOD(void,        showSearchResult,    (const std::vector<Sample>&),                   (override));
    MOCK_METHOD(void,        showRegistered,      (const std::string&),                           (override));
    MOCK_METHOD(void,        showNotFound,        (const std::string&),                           (override));
    MOCK_METHOD(void,        showError,           (const std::string&),                           (override));
};
