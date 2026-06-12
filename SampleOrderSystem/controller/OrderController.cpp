#include "OrderController.h"
#include "../util/BusinessLogic.h"
#include <ctime>
#include <sstream>
#include <iomanip>

OrderController::OrderController(SampleRepository&      sampleRepo,
                                 OrderRepository&       orderRepo,
                                 ProductionLineService& productionSvc,
                                 OrderView&             orderView,
                                 MonitorView&           monitorView,
                                 ProductionLineView&    productionLineView)
    : sampleRepo_(sampleRepo),
      orderRepo_(orderRepo),
      productionSvc_(productionSvc),
      orderView_(orderView),
      monitorView_(monitorView),
      productionLineView_(productionLineView) {}

// ── private helpers ────────────────────────────────────────────────────────

std::string OrderController::generateOrderId() {
    time_t now = time(nullptr);
    tm t{};
    localtime_s(&t, &now);
    char date[9];
    strftime(date, sizeof(date), "%Y%m%d", &t);

    int seq = orderRepo_.countByDate(date) + 1;

    std::ostringstream oss;
    oss << "ORD-" << date << "-" << std::setfill('0') << std::setw(4) << seq;
    return oss.str();
}

ProductionTask OrderController::buildProductionTask(
    const Order& order, const Sample& sample,
    int shortage, int actualProduction, double totalTime) {
    ProductionTask task;
    task.orderId          = order.id;
    task.sampleId         = sample.id;
    task.sampleName       = sample.name;
    task.orderQuantity    = order.quantity;
    task.shortage         = shortage;
    task.actualProduction = actualProduction;
    task.totalTime        = totalTime;
    task.yieldRate        = sample.yieldRate;
    return task;
}

void OrderController::approveWithSufficientStock(Order& order, Sample& sample) {
    sample.stock -= order.quantity;
    sampleRepo_.update(sample);
    order.status = OrderStatus::CONFIRMED;
}

void OrderController::approveWithProduction(Order& order, const Sample& sample,
                                            int shortage, int actualProduction,
                                            double totalTime) {
    order.prodShortage  = shortage;
    order.prodActual    = actualProduction;
    order.prodTotalTime = totalTime;
    order.prodYieldRate = sample.yieldRate;
    order.status        = OrderStatus::PRODUCING;
    // 재고 차감 없음 — PRODUCING 예약 방식
    productionSvc_.enqueue(
        buildProductionTask(order, sample, shortage, actualProduction, totalTime));
}

int OrderController::finalizeProductionTask(const ProductionTask& task) {
    auto sampleOpt = sampleRepo_.findById(task.sampleId);
    int  newStock  = 0;
    if (sampleOpt) {
        Sample sample = *sampleOpt;
        newStock      = BusinessLogic::calcStockAfterProduction(
                            sample.stock, task.actualProduction, task.orderQuantity);
        sample.stock  = newStock;
        sampleRepo_.update(sample);
    }
    auto orderOpt = orderRepo_.findById(task.orderId);
    if (orderOpt) {
        Order order  = *orderOpt;
        order.status = OrderStatus::CONFIRMED;
        orderRepo_.update(order);
    }
    return newStock;
}

// ── public actions ─────────────────────────────────────────────────────────

void OrderController::placeOrder() {
    OrderInput orderInput = orderView_.readOrderInput();
    if (orderInput.sampleId.empty() || orderInput.customerName.empty()
        || orderInput.quantity <= 0) return;

    auto sampleOpt = sampleRepo_.findById(orderInput.sampleId);
    if (!sampleOpt) {
        orderView_.showSampleNotFound(orderInput.sampleId);
        return;
    }

    if (!orderView_.confirmOrderInput(orderInput, *sampleOpt)) return;

    Order order;
    order.id           = generateOrderId();
    order.sampleId     = orderInput.sampleId;
    order.customerName = orderInput.customerName;
    order.quantity     = orderInput.quantity;
    order.status       = OrderStatus::RESERVED;

    try {
        orderRepo_.add(order);
        orderView_.showOrderPlaced(order);
    } catch (const std::exception& e) {
        orderView_.showNoOrders(e.what());
    }
}

void OrderController::processApproval() {
    while (true) {
        auto reserved = orderRepo_.findByStatus(OrderStatus::RESERVED);
        if (reserved.empty()) {
            orderView_.showNoOrders("승인 대기 중인 주문이 없습니다.");
            return;
        }

        auto allSamples    = sampleRepo_.findAll();
        int  selectedIndex = orderView_.showReservedList(reserved, allSamples);
        if (selectedIndex == 0) return;
        if (selectedIndex < 1 || selectedIndex > (int)reserved.size()) continue;

        Order  order     = reserved[selectedIndex - 1];
        auto   sampleOpt = sampleRepo_.findById(order.sampleId);
        if (!sampleOpt) continue;
        Sample sample    = *sampleOpt;

        auto   producingOrders  = orderRepo_.findByStatus(OrderStatus::PRODUCING);
        int    reservedStock    = BusinessLogic::calcReservedStock(producingOrders, sample.id);
        int    availableStock   = BusinessLogic::calcAvailableStock(sample.stock, reservedStock);
        int    shortage         = order.quantity - availableStock;
        int    actualProduction = (shortage > 0)
                                    ? BusinessLogic::calcActualProduction(shortage, sample.yieldRate)
                                    : 0;
        double totalTime        = (shortage > 0)
                                    ? BusinessLogic::calcTotalTime(sample.avgProductionTime, actualProduction)
                                    : 0.0;

        char decision = orderView_.showApprovalDetail(
            sample, order, shortage, actualProduction, totalTime);
        if (decision == '0') continue;

        if (decision == 'Y') {
            if (shortage <= 0) approveWithSufficientStock(order, sample);
            else               approveWithProduction(order, sample, shortage, actualProduction, totalTime);
        } else {
            order.status = OrderStatus::REJECTED;
        }

        orderRepo_.update(order);
        orderView_.showApprovalResult(order);
    }
}

void OrderController::processRelease() {
    while (true) {
        auto confirmed = orderRepo_.findByStatus(OrderStatus::CONFIRMED);
        if (confirmed.empty()) {
            orderView_.showNoOrders("출고 처리할 주문이 없습니다. (CONFIRMED 상태 없음)");
            return;
        }

        auto allSamples    = sampleRepo_.findAll();
        int  selectedIndex = orderView_.showConfirmedList(confirmed, allSamples);
        if (selectedIndex == 0) return;
        if (selectedIndex < 1 || selectedIndex > (int)confirmed.size()) continue;

        Order order  = confirmed[selectedIndex - 1];
        order.status = OrderStatus::RELEASED;
        orderRepo_.update(order);
        orderView_.showReleaseResult(order);
    }
}

void OrderController::showMonitoring() {
    while (true) {
        int choice = monitorView_.showSubMenu();
        switch (choice) {
            case 1: {
                auto orders  = orderRepo_.findAll();
                auto samples = sampleRepo_.findAll();
                monitorView_.showOrderStats(orders, BusinessLogic::buildStockInfoList(samples, orders));
                break;
            }
            case 2: {
                auto samples = sampleRepo_.findAll();
                auto orders  = orderRepo_.findAll();
                monitorView_.showStockStats(BusinessLogic::buildStockInfoList(samples, orders));
                break;
            }
            case 0: return;
            default: break;
        }
    }
}

void OrderController::autoCompleteFinished() {
    while (productionSvc_.hasCurrentTask()) {
        const auto& task = *productionSvc_.currentTask();
        if (task.startTime == 0)   break;
        if (task.totalTime <= 0.0) break;  // 생산시간 미확정 — 자동 완료 금지

        time_t now        = time(nullptr);
        double elapsedSec = static_cast<double>(now - task.startTime);
        double totalSec   = task.totalTime * 60.0;
        if (elapsedSec < totalSec) break;

        finalizeProductionTask(task);
        productionSvc_.completeCurrentTask();
    }
}

void OrderController::showProductionLine() {
    while (true) {
        autoCompleteFinished();

        if (productionSvc_.isEmpty()) {
            productionLineView_.showEmpty();
            return;
        }

        char userInput = productionLineView_.show(
            productionSvc_.currentTask(),
            productionSvc_.waitingQueue());

        if (userInput != 'C') return;
        if (!productionSvc_.hasCurrentTask()) continue;

        ProductionTask task     = *productionSvc_.currentTask();
        int            newStock = finalizeProductionTask(task);
        productionSvc_.completeCurrentTask();
        productionLineView_.showCompleteResult(task, newStock);
    }
}
