#include "OrderController.h"
#include <ctime>
#include <cmath>
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

// ── 비공개 유틸 ────────────────────────────────────────────────────────────

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

int OrderController::calcActualProduction(int shortage, double yieldRate) {
    return static_cast<int>(std::ceil(shortage / (yieldRate * 0.9)));
}

double OrderController::calcTotalTime(double avgTime, int actualProduction) {
    return avgTime * actualProduction;
}

// ── 공개 액션 ──────────────────────────────────────────────────────────────

void OrderController::placeOrder() {
    OrderInput in = orderView_.readOrderInput();
    if (in.sampleId.empty() || in.customerName.empty() || in.quantity <= 0) return;

    auto sampleOpt = sampleRepo_.findById(in.sampleId);
    if (!sampleOpt) {
        orderView_.showSampleNotFound(in.sampleId);
        return;
    }

    if (!orderView_.confirmOrderInput(in, *sampleOpt)) return;

    Order o;
    o.id           = generateOrderId();
    o.sampleId     = in.sampleId;
    o.customerName = in.customerName;
    o.quantity     = in.quantity;
    o.status       = OrderStatus::RESERVED;

    try {
        orderRepo_.add(o);
        orderView_.showOrderPlaced(o);
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

        auto allSamples = sampleRepo_.findAll();
        int sel = orderView_.showReservedList(reserved, allSamples);
        if (sel == 0) return;
        if (sel < 1 || sel > static_cast<int>(reserved.size())) continue;

        Order order = reserved[sel - 1];
        auto  sOpt  = sampleRepo_.findById(order.sampleId);
        if (!sOpt) continue;

        Sample sample = *sOpt;

        // 예약 방식: 이미 PRODUCING 대기 중인 주문의 선점 재고를 가용 재고에서 제외
        int reservedStock = 0;
        for (const auto& po : orderRepo_.findByStatus(OrderStatus::PRODUCING)) {
            if (po.sampleId == sample.id)
                reservedStock += (po.quantity - po.prodShortage);
        }
        int availableStock = std::max(0, sample.stock - reservedStock);

        int shortage    = order.quantity - availableStock;
        int actualProd  = 0;
        double totalTime = 0.0;

        if (shortage > 0) {
            actualProd = calcActualProduction(shortage, sample.yieldRate);
            totalTime  = calcTotalTime(sample.avgProductionTime, actualProd);
        }

        char decision = orderView_.showApprovalDetail(
            sample, order, shortage, actualProd, totalTime);

        if (decision == '0') continue;  // 취소: 상태 변경 없이 목록으로

        if (decision == 'Y') {
            if (shortage <= 0) {
                // 재고 충분 → CONFIRMED, 재고 차감
                sample.stock -= order.quantity;
                sampleRepo_.update(sample);
                order.status = OrderStatus::CONFIRMED;
            } else {
                // 재고 부족 → PRODUCING, 생산라인 투입 (재고 차감 없음 — 예약 방식)
                order.prodShortage  = shortage;
                order.prodActual    = actualProd;
                order.prodTotalTime = totalTime;
                order.prodYieldRate = sample.yieldRate;
                order.status        = OrderStatus::PRODUCING;
                ProductionTask task;
                task.orderId          = order.id;
                task.sampleId         = sample.id;
                task.sampleName       = sample.name;
                task.orderQuantity    = order.quantity;
                task.shortage         = shortage;
                task.actualProduction = actualProd;
                task.totalTime        = totalTime;
                task.yieldRate        = sample.yieldRate;
                productionSvc_.enqueue(task);
            }
        } else {
            // 'R' — 거절
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

        auto allSamples = sampleRepo_.findAll();
        int sel = orderView_.showConfirmedList(confirmed, allSamples);
        if (sel == 0) return;
        if (sel < 1 || sel > static_cast<int>(confirmed.size())) continue;

        Order order = confirmed[sel - 1];
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

                std::vector<StockInfo> stocks;
                for (const auto& s : samples) {
                    int confirmedTotal = 0;
                    int reservedStock  = 0;
                    for (const auto& o : orders) {
                        if (o.sampleId != s.id) continue;
                        if (o.status == OrderStatus::CONFIRMED)
                            confirmedTotal += o.quantity;
                        if (o.status == OrderStatus::PRODUCING)
                            reservedStock += (o.quantity - o.prodShortage);
                    }
                    std::string st = (s.stock == 0)             ? "고갈"
                                   : (s.stock < confirmedTotal) ? "부족"
                                   :                              "여유";
                    stocks.push_back({s, st, confirmedTotal, reservedStock});
                }
                monitorView_.showOrderStats(orders, stocks);
                break;
            }
            case 2: {
                auto samples = sampleRepo_.findAll();
                auto orders  = orderRepo_.findAll();

                std::vector<StockInfo> stocks;
                for (const auto& s : samples) {
                    int confirmedTotal = 0;
                    int reservedStock  = 0;
                    for (const auto& o : orders) {
                        if (o.sampleId != s.id) continue;
                        if (o.status == OrderStatus::CONFIRMED)
                            confirmedTotal += o.quantity;
                        if (o.status == OrderStatus::PRODUCING)
                            reservedStock += (o.quantity - o.prodShortage);
                    }
                    std::string st = (s.stock == 0)             ? "고갈"
                                   : (s.stock < confirmedTotal) ? "부족"
                                   :                              "여유";
                    stocks.push_back({s, st, confirmedTotal, reservedStock});
                }
                monitorView_.showStockStats(stocks);
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
        if (task.startTime == 0) break;

        time_t now        = time(nullptr);
        double elapsedSec = static_cast<double>(now - task.startTime);
        double totalSec   = task.totalTime * 60.0;
        if (elapsedSec < totalSec) break; // 아직 미완료

        // 재고 갱신
        auto sOpt = sampleRepo_.findById(task.sampleId);
        if (sOpt) {
            Sample sample = *sOpt;
            sample.stock  = sample.stock + task.actualProduction - task.orderQuantity;
            sampleRepo_.update(sample);
        }

        // 주문 상태 → CONFIRMED
        auto oOpt = orderRepo_.findById(task.orderId);
        if (oOpt) {
            Order order  = *oOpt;
            order.status = OrderStatus::CONFIRMED;
            orderRepo_.update(order);
        }

        productionSvc_.completeCurrentTask();
    }
}

void OrderController::showProductionLine() {
    while (true) {
        autoCompleteFinished(); // 진입할 때마다 자동 완료 확인

        if (productionSvc_.isEmpty()) {
            productionLineView_.showEmpty();
            return;
        }

        char c = productionLineView_.show(
            productionSvc_.currentTask(),
            productionSvc_.waitingQueue());

        if (c == 'C') {
            if (!productionSvc_.hasCurrentTask()) continue;

            ProductionTask task = *productionSvc_.currentTask();

            // 재고 갱신: 현재 재고 + 생산량 - 주문수량
            auto sOpt = sampleRepo_.findById(task.sampleId);
            if (!sOpt) { productionSvc_.completeCurrentTask(); continue; }

            Sample sample = *sOpt;
            int newStock = sample.stock + task.actualProduction - task.orderQuantity;
            sample.stock = newStock;
            sampleRepo_.update(sample);

            // 주문 상태 → CONFIRMED
            auto oOpt = orderRepo_.findById(task.orderId);
            if (oOpt) {
                Order order = *oOpt;
                order.status = OrderStatus::CONFIRMED;
                orderRepo_.update(order);
            }

            productionSvc_.completeCurrentTask();
            productionLineView_.showCompleteResult(task, newStock);
        } else {
            return;
        }
    }
}
