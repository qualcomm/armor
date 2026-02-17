// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>
#include "categorization.hpp"
#include "diff_utils.hpp"

class CategorizationTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(CategorizationTest, Changed_FatalErrors) {
    std::string result = getOverAllCategory(
        static_cast<unsigned int>(ParsedDiffStatus::FATAL_ERRORS),
        static_cast<unsigned int>(UnParsedDiffStatus::CHANGED),
        false
    );
    EXPECT_EQ(serialize(OverAllStatus::FATAL_ERRORS), result);
}

TEST_F(CategorizationTest, Changed_UnsupportedUpdates_Compatible) {
    std::string result = getOverAllCategory(
        static_cast<unsigned int>(ParsedDiffStatus::UNSUPPORTED_UPDATES),
        static_cast<unsigned int>(UnParsedDiffStatus::CHANGED),
        true
    );
    EXPECT_EQ(serialize(OverAllStatus::IN_ACTIVE), result);
}

TEST_F(CategorizationTest, Changed_UnsupportedUpdates_Incompatible) {
    std::string result = getOverAllCategory(
        static_cast<unsigned int>(ParsedDiffStatus::UNSUPPORTED_UPDATES),
        static_cast<unsigned int>(UnParsedDiffStatus::CHANGED),
        false
    );
    EXPECT_EQ(serialize(OverAllStatus::BACKWARD_INCOMPATIBLE), result);
}

TEST_F(CategorizationTest, Changed_SupportedUpdates_Compatible) {
    std::string result = getOverAllCategory(
        static_cast<unsigned int>(ParsedDiffStatus::SUPPORTED_UPDATES),
        static_cast<unsigned int>(UnParsedDiffStatus::CHANGED),
        true
    );
    EXPECT_EQ(serialize(OverAllStatus::IN_ACTIVE), result);
}

TEST_F(CategorizationTest, Changed_SupportedUpdates_Incompatible) {
    std::string result = getOverAllCategory(
        static_cast<unsigned int>(ParsedDiffStatus::SUPPORTED_UPDATES),
        static_cast<unsigned int>(UnParsedDiffStatus::CHANGED),
        false
    );
    EXPECT_EQ(serialize(OverAllStatus::BACKWARD_INCOMPATIBLE), result);
}

TEST_F(CategorizationTest, Changed_CommentsUpdated) {
    std::string result = getOverAllCategory(
        static_cast<unsigned int>(ParsedDiffStatus::COMMENTS_UPDATED),
        static_cast<unsigned int>(UnParsedDiffStatus::CHANGED),
        true
    );
    EXPECT_EQ(serialize(OverAllStatus::IN_ACTIVE), result);
}

TEST_F(CategorizationTest, Changed_NonFunctionalChanges) {
    std::string result = getOverAllCategory(
        static_cast<unsigned int>(ParsedDiffStatus::NON_FUNCTIONAL_CHANGES),
        static_cast<unsigned int>(UnParsedDiffStatus::CHANGED),
        true
    );
    EXPECT_EQ(serialize(OverAllStatus::IN_ACTIVE), result);
}

TEST_F(CategorizationTest, Changed_CommentsUpdated_Incompatible) {
    std::string result = getOverAllCategory(
        static_cast<unsigned int>(ParsedDiffStatus::COMMENTS_UPDATED),
        static_cast<unsigned int>(UnParsedDiffStatus::CHANGED),
        false
    );
    EXPECT_EQ(serialize(OverAllStatus::CATEGORIZATION_ERROR), result);
}

TEST_F(CategorizationTest, Changed_NonFunctionalChanges_Incompatible) {
    std::string result = getOverAllCategory(
        static_cast<unsigned int>(ParsedDiffStatus::NON_FUNCTIONAL_CHANGES),
        static_cast<unsigned int>(UnParsedDiffStatus::CHANGED),
        false
    );
    EXPECT_EQ(serialize(OverAllStatus::CATEGORIZATION_ERROR), result);
}

TEST_F(CategorizationTest, Unchanged_FatalErrors) {
    std::string result = getOverAllCategory(
        static_cast<unsigned int>(ParsedDiffStatus::FATAL_ERRORS),
        static_cast<unsigned int>(UnParsedDiffStatus::UN_CHANGES),
        false
    );
    EXPECT_EQ(serialize(OverAllStatus::FATAL_ERRORS), result);
}

TEST_F(CategorizationTest, Unchanged_UnsupportedUpdates_Compatible) {
    std::string result = getOverAllCategory(
        static_cast<unsigned int>(ParsedDiffStatus::UNSUPPORTED_UPDATES),
        static_cast<unsigned int>(UnParsedDiffStatus::UN_CHANGES),
        true
    );
    EXPECT_EQ(serialize(OverAllStatus::UNSUPPORTED_UPDATES), result);
}

TEST_F(CategorizationTest, Unchanged_UnsupportedUpdates_Incompatible) {
    std::string result = getOverAllCategory(
        static_cast<unsigned int>(ParsedDiffStatus::UNSUPPORTED_UPDATES),
        static_cast<unsigned int>(UnParsedDiffStatus::UN_CHANGES),
        false
    );
    EXPECT_EQ(serialize(OverAllStatus::BACKWARD_INCOMPATIBLE), result);
}

TEST_F(CategorizationTest, Unchanged_SupportedUpdates_Compatible) {
    std::string result = getOverAllCategory(
        static_cast<unsigned int>(ParsedDiffStatus::SUPPORTED_UPDATES),
        static_cast<unsigned int>(UnParsedDiffStatus::UN_CHANGES),
        true
    );
    EXPECT_EQ(serialize(OverAllStatus::BACKWARD_COMPATIABLE), result);
}

TEST_F(CategorizationTest, Unchanged_SupportedUpdates_Incompatible) {
    std::string result = getOverAllCategory(
        static_cast<unsigned int>(ParsedDiffStatus::SUPPORTED_UPDATES),
        static_cast<unsigned int>(UnParsedDiffStatus::UN_CHANGES),
        false
    );
    EXPECT_EQ(serialize(OverAllStatus::BACKWARD_INCOMPATIBLE), result);
}

TEST_F(CategorizationTest, Unchanged_CommentsUpdated) {
    std::string result = getOverAllCategory(
        static_cast<unsigned int>(ParsedDiffStatus::COMMENTS_UPDATED),
        static_cast<unsigned int>(UnParsedDiffStatus::UN_CHANGES),
        true
    );
    EXPECT_EQ(serialize(OverAllStatus::COMMENTS_UPDATED), result);
}

TEST_F(CategorizationTest, Unchanged_NonFunctionalChanges) {
    std::string result = getOverAllCategory(
        static_cast<unsigned int>(ParsedDiffStatus::NON_FUNCTIONAL_CHANGES),
        static_cast<unsigned int>(UnParsedDiffStatus::UN_CHANGES),
        true
    );
    EXPECT_EQ(serialize(OverAllStatus::NON_FUNCTIONAL_CHANGES), result);
}

TEST_F(CategorizationTest, Unchanged_CommentsUpdated_Incompatible) {
    std::string result = getOverAllCategory(
        static_cast<unsigned int>(ParsedDiffStatus::COMMENTS_UPDATED),
        static_cast<unsigned int>(UnParsedDiffStatus::UN_CHANGES),
        false
    );
    EXPECT_EQ(serialize(OverAllStatus::CATEGORIZATION_ERROR), result);
}

TEST_F(CategorizationTest, Unchanged_NonFunctionalChanges_Incompatible) {
    std::string result = getOverAllCategory(
        static_cast<unsigned int>(ParsedDiffStatus::NON_FUNCTIONAL_CHANGES),
        static_cast<unsigned int>(UnParsedDiffStatus::UN_CHANGES),
        false
    );
    EXPECT_EQ(serialize(OverAllStatus::CATEGORIZATION_ERROR), result);
}

TEST_F(CategorizationTest, InvalidParsedStatus_Changed) {
    std::string result = getOverAllCategory(
        999,
        static_cast<unsigned int>(UnParsedDiffStatus::CHANGED),
        false
    );
    EXPECT_EQ(serialize(OverAllStatus::CATEGORIZATION_ERROR), result);
}

TEST_F(CategorizationTest, InvalidParsedStatus_Unchanged) {
    std::string result = getOverAllCategory(
        999,
        static_cast<unsigned int>(UnParsedDiffStatus::UN_CHANGES),
        false
    );
    EXPECT_EQ(serialize(OverAllStatus::CATEGORIZATION_ERROR), result);
}

TEST_F(CategorizationTest, InvalidUnparsedStatus) {
    std::string result = getOverAllCategory(
        static_cast<unsigned int>(ParsedDiffStatus::SUPPORTED_UPDATES),
        999,
        false
    );
    EXPECT_EQ(serialize(OverAllStatus::CATEGORIZATION_ERROR), result);
}