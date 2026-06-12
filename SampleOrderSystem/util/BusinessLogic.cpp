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

} // namespace BusinessLogic
