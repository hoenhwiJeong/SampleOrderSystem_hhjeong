#pragma once
#include "../model/Order.h"
#include "../model/Sample.h"
#include <vector>
#include <string>

struct OrderInput {
    std::string sampleId;
    std::string customerName;
    int         quantity;
};

class OrderView {
public:
    OrderInput readOrderInput();
    bool       confirmOrderInput(const OrderInput& in, const Sample& s);
    void       showOrderPlaced(const Order& o);
    // returns: 1-based index of selected order, 0=back
    int        showReservedList(const std::vector<Order>& orders,
                                const std::vector<Sample>& samples);
    // returns: true=approve, false=reject
    bool       showApprovalDetail(const Sample& s, const Order& o,
                                  int shortage, int actualProd, double totalTime);
    void       showApprovalResult(const Order& o);
    // returns: 1-based index of selected order, 0=back
    int        showConfirmedList(const std::vector<Order>& orders,
                                 const std::vector<Sample>& samples);
    void       showReleaseResult(const Order& o);
    void       showNoOrders(const std::string& msg);
    void       showSampleNotFound(const std::string& id);
};
