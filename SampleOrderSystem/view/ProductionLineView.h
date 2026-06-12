#pragma once
#include "../service/ProductionLineService.h"
#include <optional>
#include <queue>

class ProductionLineView {
public:
    virtual ~ProductionLineView() = default;

    // returns: 'C'=complete current, '0'=back
    virtual char show(const std::optional<ProductionTask>& current,
                      const std::queue<ProductionTask>&    waiting);
    virtual void showCompleteResult(const ProductionTask& task, int newStock);
    virtual void showEmpty();
};
