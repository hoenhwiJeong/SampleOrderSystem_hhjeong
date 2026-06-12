#include "BusinessLogic.h"

namespace BusinessLogic {

int calcActualProduction(int shortage, double yieldRate) {
    return static_cast<int>(std::ceil(shortage / (yieldRate * DEFECT_RATE_MARGIN)));
}

double calcTotalTime(double avgProductionTime, int actualProduction) {
    return avgProductionTime * actualProduction;
}

int calcReservedStock(const std::vector<Order>& orders,
                      const std::string& sampleId) {
    int reserved = 0;
    for (const auto& o : orders) {
        if (o.sampleId == sampleId && o.status == OrderStatus::PRODUCING)
            reserved += (o.quantity - o.prodShortage);
    }
    return reserved;
}

int calcAvailableStock(int currentStock, int reservedStock) {
    return std::max(0, currentStock - reservedStock);
}

int calcStockAfterProduction(int currentStock,
                             int actualProduction,
                             int orderQuantity) {
    return currentStock + actualProduction - orderQuantity;
}

std::vector<StockInfo> buildStockInfoList(const std::vector<Sample>& samples,
                                          const std::vector<Order>&  orders) {
    std::vector<StockInfo> result;
    for (const auto& sample : samples) {
        int confirmedTotal = 0;
        int reservedStock  = 0;
        for (const auto& order : orders) {
            if (order.sampleId != sample.id) continue;
            if (order.status == OrderStatus::CONFIRMED)
                confirmedTotal += order.quantity;
            if (order.status == OrderStatus::PRODUCING)
                reservedStock += (order.quantity - order.prodShortage);
        }
        std::string status = (sample.stock == 0)             ? "고갈"
                           : (sample.stock < confirmedTotal) ? "부족"
                           :                                   "여유";
        result.push_back({sample, status, confirmedTotal, reservedStock});
    }
    return result;
}

} // namespace BusinessLogic
