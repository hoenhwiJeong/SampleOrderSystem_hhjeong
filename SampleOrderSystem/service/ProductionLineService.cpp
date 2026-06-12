#include "ProductionLineService.h"

void ProductionLineService::enqueue(const ProductionTask& task) {
    if (!current_.has_value()) {
        current_ = task;
        current_->startTime = time(nullptr); // 즉시 생산 시작
    } else {
        queue_.push(task);
    }
}

std::optional<ProductionTask> ProductionLineService::currentTask() const {
    return current_;
}

std::queue<ProductionTask> ProductionLineService::waitingQueue() const {
    return queue_;
}

bool ProductionLineService::isEmpty() const {
    return !current_.has_value() && queue_.empty();
}

bool ProductionLineService::hasCurrentTask() const {
    return current_.has_value();
}

void ProductionLineService::completeCurrentTask() {
    if (!queue_.empty()) {
        current_ = queue_.front();
        current_->startTime = time(nullptr); // 다음 작업 시작 시각 기록
        queue_.pop();
    } else {
        current_ = std::nullopt;
    }
}
