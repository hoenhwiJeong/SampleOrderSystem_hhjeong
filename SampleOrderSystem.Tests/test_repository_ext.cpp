#include <gtest/gtest.h>
#include "repository/SampleRepository.h"
#include "repository/OrderRepository.h"
#include <filesystem>

// ── SampleRepository::findByName ─────────────────────────────────────────────

class FindByNameTest : public ::testing::Test {
protected:
    const std::string path = "test_tmp_findbyname.json";

    void SetUp()    override { std::filesystem::remove(path); }
    void TearDown() override { std::filesystem::remove(path); }

    void addSample(const std::string& id, const std::string& name) {
        SampleRepository repo(path);
        Sample s;
        s.id = id; s.name = name;
        s.avgProductionTime = 0.5; s.yieldRate = 0.9; s.stock = 100;
        repo.add(s);
    }
};

TEST_F(FindByNameTest, KoreanKeyword_ReturnsMatch) {
    addSample("S-T01", "실리콘 웨이퍼-8인치");
    addSample("S-T02", "GaN 에피택셜-4인치");
    addSample("S-T03", "SiC 파워기판-6인치");

    SampleRepository repo(path);
    auto result = repo.findByName("웨이퍼");
    EXPECT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].id, "S-T01");
}

TEST_F(FindByNameTest, PartialMatch_ReturnsAll) {
    addSample("S-T01", "산화막 웨이퍼-SiO2");
    addSample("S-T02", "실리콘 웨이퍼-8인치");
    addSample("S-T03", "포토레지스트-PR7");

    SampleRepository repo(path);
    auto result = repo.findByName("웨이퍼");
    EXPECT_EQ(result.size(), 2u);
}

TEST_F(FindByNameTest, NoMatch_ReturnsEmptyVector) {
    addSample("S-T01", "실리콘 웨이퍼-8인치");

    SampleRepository repo(path);
    auto result = repo.findByName("존재하지않는키워드");
    EXPECT_TRUE(result.empty());
}

// ── OrderRepository::countByDate ─────────────────────────────────────────────

class CountByDateTest : public ::testing::Test {
protected:
    const std::string path = "test_tmp_countdate.json";

    void SetUp()    override { std::filesystem::remove(path); }
    void TearDown() override { std::filesystem::remove(path); }

    void addOrder(const std::string& id) {
        OrderRepository repo(path);
        Order o;
        o.id = id; o.sampleId = "S-001"; o.customerName = "고객";
        o.quantity = 10; o.status = OrderStatus::RESERVED;
        repo.add(o);
    }
};

TEST_F(CountByDateTest, SameDateOrders_CountsCorrectly) {
    addOrder("ORD-20260612-0001");
    addOrder("ORD-20260612-0002");
    addOrder("ORD-20260612-0003");

    OrderRepository repo(path);
    EXPECT_EQ(repo.countByDate("20260612"), 3);
}

TEST_F(CountByDateTest, DifferentDate_NotCounted) {
    addOrder("ORD-20260612-0001");
    addOrder("ORD-20260613-0001");

    OrderRepository repo(path);
    EXPECT_EQ(repo.countByDate("20260612"), 1);
    EXPECT_EQ(repo.countByDate("20260613"), 1);
}
