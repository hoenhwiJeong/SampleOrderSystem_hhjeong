#pragma once
#include "../repository/SampleRepository.h"
#include "../repository/OrderRepository.h"
#include "../service/ProductionLineService.h"
#include "../view/OrderView.h"
#include "../view/MonitorView.h"
#include "../view/ProductionLineView.h"

class OrderController {
public:
    OrderController(SampleRepository&      sampleRepo,
                    OrderRepository&       orderRepo,
                    ProductionLineService& productionSvc,
                    OrderView&             orderView,
                    MonitorView&           monitorView,
                    ProductionLineView&    productionLineView);

    void placeOrder();
    void processApproval();
    void processRelease();
    void showMonitoring();
    void showProductionLine();
    void autoCompleteFinished(); // 완료된 생산 작업 자동 처리

private:
    SampleRepository&      sampleRepo_;
    OrderRepository&       orderRepo_;
    ProductionLineService& productionSvc_;
    OrderView&             orderView_;
    MonitorView&           monitorView_;
    ProductionLineView&    productionLineView_;

    std::string generateOrderId();
};
