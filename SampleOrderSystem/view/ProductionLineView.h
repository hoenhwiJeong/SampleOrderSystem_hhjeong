#pragma once
#include "../service/ProductionLineService.h"
#include <optional>
#include <queue>

class ProductionLineView {
public:
    // returns: 'C'=complete current, '0'=back
    char show(const std::optional<ProductionTask>& current,
              const std::queue<ProductionTask>&    waiting);
    void showCompleteResult(const ProductionTask& task, int newStock);
    void showEmpty();
};
