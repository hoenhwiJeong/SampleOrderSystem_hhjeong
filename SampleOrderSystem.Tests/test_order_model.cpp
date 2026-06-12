#include <gtest/gtest.h>
#include "model/Order.h"

// toJson → fromJson 왕복 후 기본 필드 일치
TEST(OrderModel, SerializeDeserializeRoundTrip) {
    Order o;
    o.id           = "ORD-20260612-0001";
    o.sampleId     = "S-001";
    o.customerName = "삼성전자";
    o.quantity     = 100;
    o.status       = OrderStatus::RESERVED;

    auto rebuilt = Order::fromJson(o.toJson());

    EXPECT_EQ(rebuilt.id,           o.id);
    EXPECT_EQ(rebuilt.sampleId,     o.sampleId);
    EXPECT_EQ(rebuilt.customerName, o.customerName);
    EXPECT_EQ(rebuilt.quantity,     o.quantity);
    EXPECT_EQ(rebuilt.status,       OrderStatus::RESERVED);
}

// PRODUCING 상태: prod 필드가 직렬화되어 복원됨
TEST(OrderModel, ProducingFieldsSerializedAndRestored) {
    Order o;
    o.id           = "ORD-20260612-0002";
    o.sampleId     = "S-003";
    o.customerName = "하이닉스";
    o.quantity     = 80;
    o.status       = OrderStatus::PRODUCING;
    o.prodShortage  = 50;
    o.prodActual    = 61;
    o.prodTotalTime = 48.8;
    o.prodYieldRate = 0.92;

    auto rebuilt = Order::fromJson(o.toJson());

    EXPECT_EQ(rebuilt.prodShortage,         50);
    EXPECT_EQ(rebuilt.prodActual,           61);
    EXPECT_DOUBLE_EQ(rebuilt.prodTotalTime, 48.8);
    EXPECT_DOUBLE_EQ(rebuilt.prodYieldRate, 0.92);
}

// CONFIRMED 상태: prod 필드는 저장되지 않으므로 기본값(0) 복원
TEST(OrderModel, NonProducingProdFieldsNotSerialized) {
    Order o;
    o.status       = OrderStatus::CONFIRMED;
    o.prodShortage  = 99;
    o.prodActual    = 99;

    auto rebuilt = Order::fromJson(o.toJson());

    EXPECT_EQ(rebuilt.prodShortage, 0);
    EXPECT_EQ(rebuilt.prodActual,   0);
}

// statusToString / statusFromString 왕복
TEST(OrderModel, StatusStringRoundTrip) {
    for (auto s : { OrderStatus::RESERVED, OrderStatus::PRODUCING,
                    OrderStatus::CONFIRMED, OrderStatus::RELEASED,
                    OrderStatus::REJECTED }) {
        EXPECT_EQ(Order::statusFromString(Order::statusToString(s)), s);
    }
}

// 알 수 없는 상태 문자열 → runtime_error
TEST(OrderModel, UnknownStatusThrows) {
    EXPECT_THROW(Order::statusFromString("UNKNOWN"), std::runtime_error);
}
