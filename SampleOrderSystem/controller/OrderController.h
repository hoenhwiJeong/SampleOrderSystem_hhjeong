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
    void autoCompleteFinished();

private:
    SampleRepository&      sampleRepo_;
    OrderRepository&       orderRepo_;
    ProductionLineService& productionSvc_;
    OrderView&             orderView_;
    MonitorView&           monitorView_;
    ProductionLineView&    productionLineView_;

    std::string    generateOrderId();

    // 승인 처리 헬퍼
    ProductionTask buildProductionTask(const Order& order, const Sample& sample,
                                       int shortage, int actualProduction, double totalTime);
    void           approveWithSufficientStock(Order& order, Sample& sample);
    void           approveWithProduction(Order& order, const Sample& sample,
                                         int shortage, int actualProduction, double totalTime);

    // 생산 완료 처리: 재고·주문 상태 갱신, 갱신된 재고 반환
    int finalizeProductionTask(const ProductionTask& task);
};
