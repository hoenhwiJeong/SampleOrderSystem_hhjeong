#pragma once
#include <string>
#include <queue>
#include <optional>

struct ProductionTask {
    std::string orderId;
    std::string sampleId;
    std::string sampleName;
    int         orderQuantity;
    int         shortage;
    int         actualProduction;
    double      totalTime;          // min
    double      yieldRate;          // 0.0 ~ 1.0
};

class ProductionLineService {
public:
    void                                  enqueue(const ProductionTask& task);
    std::optional<ProductionTask>         currentTask() const;
    std::queue<ProductionTask>            waitingQueue() const;
    bool                                  isEmpty() const;
    bool                                  hasCurrentTask() const;
    void                                  completeCurrentTask();

private:
    std::optional<ProductionTask> current_;
    std::queue<ProductionTask>    queue_;
};
