#include <gtest/gtest.h>
#include "repository/SampleRepository.h"
#include "repository/OrderRepository.h"
#include <filesystem>

// ── SampleRepository ──────────────────────────────────────────────────────

class SampleRepositoryTest : public ::testing::Test {
protected:
    const std::string path = "test_tmp_samples.json";

    void SetUp()    override { std::filesystem::remove(path); }
    void TearDown() override { std::filesystem::remove(path); }

    Sample makeSample(const std::string& id, int stock = 100) {
        Sample s;
        s.id = id; s.name = "테스트시료-" + id;
        s.avgProductionTime = 0.5; s.yieldRate = 0.9; s.stock = stock;
        return s;
    }
};

TEST_F(SampleRepositoryTest, AddAndFindById) {
    SampleRepository repo(path);
    repo.add(makeSample("S-T01", 100));

    auto found = repo.findById("S-T01");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name,  "테스트시료-S-T01");
    EXPECT_EQ(found->stock, 100);
}

TEST_F(SampleRepositoryTest, FindByIdMissingReturnsNullopt) {
    SampleRepository repo(path);
    EXPECT_FALSE(repo.findById("NONEXISTENT").has_value());
}

TEST_F(SampleRepositoryTest, DuplicateIdThrows) {
    SampleRepository repo(path);
    repo.add(makeSample("S-T01"));
    EXPECT_THROW(repo.add(makeSample("S-T01")), std::runtime_error);
}

TEST_F(SampleRepositoryTest, UpdateChangesStock) {
    SampleRepository repo(path);
    repo.add(makeSample("S-T01", 100));

    auto s  = *repo.findById("S-T01");
    s.stock = 200;
    repo.update(s);

    EXPECT_EQ(repo.findById("S-T01")->stock, 200);
}

// 재로드 후에도 변경 사항이 유지되는지 (영속성)
TEST_F(SampleRepositoryTest, UpdatePersistsAcrossReload) {
    {
        SampleRepository repo(path);
        repo.add(makeSample("S-T01", 100));
        auto s = *repo.findById("S-T01"); s.stock = 999;
        repo.update(s);
    }
    SampleRepository repo2(path);
    EXPECT_EQ(repo2.findById("S-T01")->stock, 999);
}

TEST_F(SampleRepositoryTest, FindAllReturnsAllAdded) {
    SampleRepository repo(path);
    repo.add(makeSample("S-T01"));
    repo.add(makeSample("S-T02"));
    repo.add(makeSample("S-T03"));

    EXPECT_EQ(repo.findAll().size(), 3u);
}

// ── OrderRepository ───────────────────────────────────────────────────────

class OrderRepositoryTest : public ::testing::Test {
protected:
    const std::string path = "test_tmp_orders.json";

    void SetUp()    override { std::filesystem::remove(path); }
    void TearDown() override { std::filesystem::remove(path); }

    Order makeOrder(const std::string& id, OrderStatus status = OrderStatus::RESERVED) {
        Order o;
        o.id = id; o.sampleId = "S-001"; o.customerName = "테스트고객";
        o.quantity = 10; o.status = status;
        return o;
    }
};

TEST_F(OrderRepositoryTest, AddAndFindById) {
    OrderRepository repo(path);
    repo.add(makeOrder("ORD-T001"));

    auto found = repo.findById("ORD-T001");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->customerName, "테스트고객");
}

TEST_F(OrderRepositoryTest, FindByStatusFiltersCorrectly) {
    OrderRepository repo(path);
    repo.add(makeOrder("ORD-T001", OrderStatus::RESERVED));
    repo.add(makeOrder("ORD-T002", OrderStatus::CONFIRMED));
    repo.add(makeOrder("ORD-T003", OrderStatus::RESERVED));

    auto reserved = repo.findByStatus(OrderStatus::RESERVED);
    EXPECT_EQ(reserved.size(), 2u);

    auto confirmed = repo.findByStatus(OrderStatus::CONFIRMED);
    EXPECT_EQ(confirmed.size(), 1u);
}

TEST_F(OrderRepositoryTest, UpdateStatusPersists) {
    {
        OrderRepository repo(path);
        repo.add(makeOrder("ORD-T001", OrderStatus::RESERVED));
        auto o  = *repo.findById("ORD-T001");
        o.status = OrderStatus::CONFIRMED;
        repo.update(o);
    }
    OrderRepository repo2(path);
    EXPECT_EQ(repo2.findById("ORD-T001")->status, OrderStatus::CONFIRMED);
}

// PRODUCING 주문의 prod 필드가 영속화되는지
TEST_F(OrderRepositoryTest, ProducingFieldsPersist) {
    {
        OrderRepository repo(path);
        Order o = makeOrder("ORD-T001", OrderStatus::PRODUCING);
        o.prodShortage  = 50;
        o.prodActual    = 61;
        o.prodTotalTime = 48.8;
        o.prodYieldRate = 0.92;
        repo.add(o);
    }
    OrderRepository repo2(path);
    auto found = repo2.findById("ORD-T001");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->prodShortage,          50);
    EXPECT_EQ(found->prodActual,            61);
    EXPECT_DOUBLE_EQ(found->prodTotalTime,  48.8);
}
