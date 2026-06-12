#pragma once
#include <gmock/gmock.h>
#include "view/OrderView.h"

class MockOrderView : public OrderView {
public:
    MOCK_METHOD(OrderInput, readOrderInput,      (),                                                           (override));
    MOCK_METHOD(bool,       confirmOrderInput,   (const OrderInput&, const Sample&),                          (override));
    MOCK_METHOD(void,       showOrderPlaced,     (const Order&),                                              (override));
    MOCK_METHOD(int,        showReservedList,    (const std::vector<Order>&, const std::vector<Sample>&),     (override));
    MOCK_METHOD(char,       showApprovalDetail, (const Sample&, const Order&, int, int, double),              (override));
    MOCK_METHOD(void,       showApprovalResult, (const Order&),                                               (override));
    MOCK_METHOD(int,        showConfirmedList,  (const std::vector<Order>&, const std::vector<Sample>&),      (override));
    MOCK_METHOD(void,       showReleaseResult,  (const Order&),                                               (override));
    MOCK_METHOD(void,       showNoOrders,       (const std::string&),                                         (override));
    MOCK_METHOD(void,       showSampleNotFound, (const std::string&),                                         (override));
};
