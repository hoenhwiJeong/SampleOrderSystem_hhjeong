#pragma once
#include <gmock/gmock.h>
#include "view/ProductionLineView.h"

class MockProductionLineView : public ProductionLineView {
public:
    MOCK_METHOD(char, show,               (const std::optional<ProductionTask>&,
                                           const std::queue<ProductionTask>&),   (override));
    MOCK_METHOD(void, showCompleteResult, (const ProductionTask&, int),          (override));
    MOCK_METHOD(void, showEmpty,          (),                                    (override));
};
