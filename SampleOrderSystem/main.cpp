#include <iostream>
#include <Windows.h>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "util/ConsoleUI.h"
#include "util/JsonHelper.h"
#include "model/Sample.h"
#include "model/Order.h"
#include "repository/SampleRepository.h"
#include "repository/OrderRepository.h"
#include "service/ProductionLineService.h"
#include "view/MainView.h"
#include "view/SampleView.h"
#include "view/OrderView.h"
#include "view/MonitorView.h"
#include "view/ProductionLineView.h"
#include "controller/SampleController.h"
#include "controller/OrderController.h"

static std::string nowString() {
    time_t now = time(nullptr);
    tm t{};
    localtime_s(&t, &now);
    std::ostringstream oss;
    oss << std::put_time(&t, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

static SystemSummary buildSummary(SampleRepository& sRepo,
                                   OrderRepository&  oRepo) {
    SystemSummary s{};
    auto samples = sRepo.findAll();
    s.sampleCount = static_cast<int>(samples.size());
    for (const auto& sample : samples) s.totalStock += sample.stock;

    auto orders = oRepo.findAll();
    for (const auto& o : orders) {
        if (o.status != OrderStatus::REJECTED) ++s.totalOrders;
        if (o.status == OrderStatus::PRODUCING) ++s.producingCount;
        if (o.status == OrderStatus::RESERVED)  ++s.reservedCount;
    }
    s.currentTime = nowString();
    return s;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // ── 레포지토리 초기화 ─────────────────────────────────
    SampleRepository sampleRepo("data/samples.json");
    OrderRepository  orderRepo ("data/orders.json");

    // ── 서비스 ────────────────────────────────────────────
    ProductionLineService productionSvc;

    // PRODUCING 주문 → 생산라인 큐 복원 (앱 재시작 시 인메모리 큐 복구)
    {
        auto producing = orderRepo.findByStatus(OrderStatus::PRODUCING);
        for (auto o : producing) {
            auto sOpt = sampleRepo.findById(o.sampleId);

            // prod 필드가 없는 구형 데이터 → 시료 데이터로 재계산
            if (o.prodTotalTime <= 0.0 && sOpt) {
                int shortage    = o.quantity; // 보수적으로 전량 부족 가정
                int actualProd  = static_cast<int>(
                    std::ceil(shortage / (sOpt->yieldRate * 0.9)));
                o.prodShortage  = shortage;
                o.prodActual    = actualProd;
                o.prodTotalTime = sOpt->avgProductionTime * actualProd;
                o.prodYieldRate = sOpt->yieldRate;
                orderRepo.update(o); // DB에 반영
            }

            ProductionTask task;
            task.orderId          = o.id;
            task.sampleId         = o.sampleId;
            task.sampleName       = sOpt ? sOpt->name : o.sampleId;
            task.orderQuantity    = o.quantity;
            task.shortage         = o.prodShortage;
            task.actualProduction = o.prodActual;
            task.totalTime        = o.prodTotalTime;
            task.yieldRate        = o.prodYieldRate;
            task.startTime        = time(nullptr); // 재시작 시 처음부터 카운트
            productionSvc.enqueue(task);
        }
    }

    // ── 뷰 ───────────────────────────────────────────────
    MainView          mainView;
    SampleView        sampleView;
    OrderView         orderView;
    MonitorView       monitorView;
    ProductionLineView productionLineView;

    // ── 컨트롤러 ─────────────────────────────────────────
    SampleController sampleCtrl(sampleRepo, sampleView);
    OrderController  orderCtrl (sampleRepo, orderRepo, productionSvc,
                                orderView, monitorView, productionLineView);

    // ── 메인 루프 ─────────────────────────────────────────
    while (true) {
        orderCtrl.autoCompleteFinished(); // 완료된 생산 작업 자동 처리
        SystemSummary summary = buildSummary(sampleRepo, orderRepo);
        int choice = mainView.showMenu(summary);

        switch (choice) {
            case 1: sampleCtrl.handleMenu();        break;
            case 2: orderCtrl.placeOrder();          break;
            case 3: orderCtrl.processApproval();     break;
            case 4: orderCtrl.showMonitoring();      break;
            case 5: orderCtrl.showProductionLine();  break;
            case 6: orderCtrl.processRelease();      break;
            case 0:
                ConsoleUI::clearScreen();
                std::cout << "  S-Semi 시스템을 종료합니다.\n\n";
                return 0;
            default:
                break;
        }
    }
}
