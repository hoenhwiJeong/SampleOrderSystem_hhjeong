#include <gtest/gtest.h>
#include "model/Sample.h"

static Sample makeSample(const std::string& id = "S-001",
                         const std::string& name = "테스트시료",
                         double time = 0.5, double yield = 0.92, int stock = 100) {
    Sample s;
    s.id                = id;
    s.name              = name;
    s.avgProductionTime = time;
    s.yieldRate         = yield;
    s.stock             = stock;
    return s;
}

// ── toJson / fromJson 라운드트립 ───────────────────────────────────────────

TEST(SampleModel, RoundTrip_AllFields) {
    Sample orig = makeSample("S-003", "SiC 파워기판-6인치", 0.8, 0.92, 30);
    Sample copy = Sample::fromJson(orig.toJson());

    EXPECT_EQ(copy.id,                orig.id);
    EXPECT_EQ(copy.name,              orig.name);
    EXPECT_DOUBLE_EQ(copy.avgProductionTime, orig.avgProductionTime);
    EXPECT_DOUBLE_EQ(copy.yieldRate,  orig.yieldRate);
    EXPECT_EQ(copy.stock,             orig.stock);
}

TEST(SampleModel, ToJson_FieldNamesCorrect) {
    Sample s = makeSample();
    auto j = s.toJson();

    EXPECT_TRUE(j.contains("id"));
    EXPECT_TRUE(j.contains("name"));
    EXPECT_TRUE(j.contains("avgProductionTime"));
    EXPECT_TRUE(j.contains("yieldRate"));
    EXPECT_TRUE(j.contains("stock"));
}

// stock=0 (산화막 웨이퍼 케이스)
TEST(SampleModel, RoundTrip_StockZero) {
    Sample s = makeSample("S-005", "산화막 웨이퍼-SiO2", 0.6, 0.88, 0);
    Sample copy = Sample::fromJson(s.toJson());

    EXPECT_EQ(copy.stock, 0);
}

// yieldRate 소수점 정밀도 보존
TEST(SampleModel, RoundTrip_YieldRatePrecision) {
    Sample s = makeSample("S-001", "실리콘 웨이퍼", 0.5, 0.92, 480);
    Sample copy = Sample::fromJson(s.toJson());

    EXPECT_DOUBLE_EQ(copy.yieldRate, 0.92);
    EXPECT_DOUBLE_EQ(copy.avgProductionTime, 0.5);
}

// 필수 필드 누락 시 예외 발생
TEST(SampleModel, FromJson_MissingFieldThrows) {
    nlohmann::json j = {{"id", "S-T"}, {"name", "시료"}};
    // avgProductionTime, yieldRate, stock 누락
    EXPECT_THROW(Sample::fromJson(j), nlohmann::json::out_of_range);
}
