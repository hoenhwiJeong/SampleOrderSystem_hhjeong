#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <filesystem>
#include "controller/SampleController.h"
#include "mock/MockSampleView.h"

using ::testing::Return;
using ::testing::_;
using ::testing::NiceMock;

static SampleInput makeInput(const std::string& id = "S-T01",
                              double yield = 0.9, int stock = 100) {
    return {id, "테스트시료", 0.5, yield, stock};
}

class SampleControllerTest : public ::testing::Test {
protected:
    const std::string path = "test_tmp_sc.json";

    void SetUp()    override { std::filesystem::remove(path); }
    void TearDown() override { std::filesystem::remove(path); }

    // NiceMock: 명시하지 않은 호출(showRegistered 등)에 경고 없이 기본값 반환
    NiceMock<MockSampleView> mockView;
    SampleRepository         repo{path};

    // 헬퍼: showSubMenu가 choice 한 번 반환 후 0으로 종료하도록 설정
    void expectMenuSequence(int choice) {
        EXPECT_CALL(mockView, showSubMenu())
            .WillOnce(Return(choice))
            .WillOnce(Return(0));
    }
};

// ── registerSample 유효성 검사 ─────────────────────────────────────────────

TEST_F(SampleControllerTest, RegisterSample_ValidInput_AddedToRepo) {
    SampleController ctrl(repo, mockView);
    expectMenuSequence(1);
    EXPECT_CALL(mockView, readSampleInput()).WillOnce(Return(makeInput("S-T01")));
    EXPECT_CALL(mockView, confirmSampleInput(_)).WillOnce(Return(true));

    ctrl.handleMenu();

    ASSERT_EQ(repo.findAll().size(), 1u);
    EXPECT_EQ(repo.findById("S-T01")->name, "테스트시료");
}

TEST_F(SampleControllerTest, RegisterSample_InvalidYieldRate_NotAdded) {
    SampleController ctrl(repo, mockView);
    expectMenuSequence(1);
    EXPECT_CALL(mockView, readSampleInput()).WillOnce(Return(makeInput("S-T01", 1.5)));
    EXPECT_CALL(mockView, confirmSampleInput(_)).WillOnce(Return(true));

    ctrl.handleMenu();

    EXPECT_TRUE(repo.findAll().empty());
}

TEST_F(SampleControllerTest, RegisterSample_ZeroYieldRate_NotAdded) {
    SampleController ctrl(repo, mockView);
    expectMenuSequence(1);
    EXPECT_CALL(mockView, readSampleInput()).WillOnce(Return(makeInput("S-T01", 0.0)));
    EXPECT_CALL(mockView, confirmSampleInput(_)).WillOnce(Return(true));

    ctrl.handleMenu();

    EXPECT_TRUE(repo.findAll().empty());
}

TEST_F(SampleControllerTest, RegisterSample_EmptyId_NotAdded) {
    SampleController ctrl(repo, mockView);
    expectMenuSequence(1);
    EXPECT_CALL(mockView, readSampleInput()).WillOnce(Return(makeInput("")));
    EXPECT_CALL(mockView, confirmSampleInput(_)).WillOnce(Return(true));

    ctrl.handleMenu();

    EXPECT_TRUE(repo.findAll().empty());
}

TEST_F(SampleControllerTest, RegisterSample_NegativeStock_NotAdded) {
    SampleController ctrl(repo, mockView);
    expectMenuSequence(1);
    SampleInput badInput{"S-T01", "테스트", 0.5, 0.9, -1};
    EXPECT_CALL(mockView, readSampleInput()).WillOnce(Return(badInput));
    EXPECT_CALL(mockView, confirmSampleInput(_)).WillOnce(Return(true));

    ctrl.handleMenu();

    EXPECT_TRUE(repo.findAll().empty());
}

TEST_F(SampleControllerTest, RegisterSample_DuplicateId_ShowsError) {
    // 미리 동일 ID 등록
    Sample existing;
    existing.id = "S-T01"; existing.name = "기존시료";
    existing.avgProductionTime = 0.5; existing.yieldRate = 0.9; existing.stock = 100;
    repo.add(existing);

    SampleController ctrl(repo, mockView);
    expectMenuSequence(1);
    EXPECT_CALL(mockView, readSampleInput()).WillOnce(Return(makeInput("S-T01")));
    EXPECT_CALL(mockView, confirmSampleInput(_)).WillOnce(Return(true));
    EXPECT_CALL(mockView, showError(_)).Times(1);

    ctrl.handleMenu();

    // 중복 삽입 방어: 여전히 1개만 존재
    EXPECT_EQ(repo.findAll().size(), 1u);
}

// ── listSamples 빈 repo ────────────────────────────────────────────────────

TEST_F(SampleControllerTest, ListSamples_EmptyRepo_ShowsNotFound) {
    SampleController ctrl(repo, mockView);
    expectMenuSequence(2); // 2 → listSamples

    EXPECT_CALL(mockView, showNotFound(_)).Times(1);

    ctrl.handleMenu();
}
