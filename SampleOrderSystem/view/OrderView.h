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
    virtual ~OrderView() = default;

    virtual OrderInput readOrderInput();
    virtual bool       confirmOrderInput(const OrderInput& in, const Sample& s);
    virtual void       showOrderPlaced(const Order& o);
    // returns: 1-based index of selected order, 0=back
    virtual int        showReservedList(const std::vector<Order>& orders,
                                        const std::vector<Sample>& samples);
    // returns: 'Y'=approve, 'R'=reject, '0'=cancel
    virtual char       showApprovalDetail(const Sample& s, const Order& o,
                                          int shortage, int actualProd, double totalTime);
    virtual void       showApprovalResult(const Order& o);
    // returns: 1-based index of selected order, 0=back
    virtual int        showConfirmedList(const std::vector<Order>& orders,
                                         const std::vector<Sample>& samples);
    virtual void       showReleaseResult(const Order& o);
    virtual void       showNoOrders(const std::string& msg);
    virtual void       showSampleNotFound(const std::string& id);
};
