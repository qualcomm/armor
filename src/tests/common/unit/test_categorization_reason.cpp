// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>
#include "categorization.hpp"
#include "diff_utils.hpp"

class CategorizationReasonTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(CategorizationReasonTest, Changed_FatalErrors) {
    const char* result = getReasonForCategorization(
        static_cast<unsigned int>(ParsedDiffStatus::FATAL_ERRORS),
        static_cast<unsigned int>(UnParsedDiffStatus::CHANGED),
        false
    );

    EXPECT_STREQ(CRITICAL_PARSING_ERRORS, result);
}

TEST_F(CategorizationReasonTest, Changed_UnsupportedUpdates_Compatible) {
    const char* result = getReasonForCategorization(
        static_cast<unsigned int>(ParsedDiffStatus::UNSUPPORTED_UPDATES),
        static_cast<unsigned int>(UnParsedDiffStatus::CHANGED),
        true
    );

    EXPECT_STREQ(CHANGED_UNSUPPORTED_COMPATIBLE, result);
}

TEST_F(CategorizationReasonTest, Changed_UnsupportedUpdates_Incompatible) {
    const char* result = getReasonForCategorization(
        static_cast<unsigned int>(ParsedDiffStatus::UNSUPPORTED_UPDATES),
        static_cast<unsigned int>(UnParsedDiffStatus::CHANGED),
        false
    );

    EXPECT_STREQ(CHANGED_UNSUPPORTED_INCOMPATIBLE, result);
}

TEST_F(CategorizationReasonTest, Changed_SupportedUpdates_Compatible) {
    const char* result = getReasonForCategorization(
        static_cast<unsigned int>(ParsedDiffStatus::SUPPORTED_UPDATES),
        static_cast<unsigned int>(UnParsedDiffStatus::CHANGED),
        true
    );

    EXPECT_STREQ(CHANGED_SUPPORTED_COMPATIBLE, result);
}

TEST_F(CategorizationReasonTest, Changed_SupportedUpdates_Incompatible) {
    const char* result = getReasonForCategorization(
        static_cast<unsigned int>(ParsedDiffStatus::SUPPORTED_UPDATES),
        static_cast<unsigned int>(UnParsedDiffStatus::CHANGED),
        false
    );

    EXPECT_STREQ(CHANGED_SUPPORTED_INCOMPATIBLE, result);
}

TEST_F(CategorizationReasonTest, Changed_CommentsUpdated) {
    const char* result = getReasonForCategorization(
        static_cast<unsigned int>(ParsedDiffStatus::COMMENTS_UPDATED),
        static_cast<unsigned int>(UnParsedDiffStatus::CHANGED),
        true
    );

    EXPECT_STREQ(CHANGED_COMMENTS, result);
}

TEST_F(CategorizationReasonTest, Changed_NonFunctionalChanges) {
    const char* result = getReasonForCategorization(
        static_cast<unsigned int>(ParsedDiffStatus::NON_FUNCTIONAL_CHANGES),
        static_cast<unsigned int>(UnParsedDiffStatus::CHANGED),
        true
    );

    EXPECT_STREQ(CHANGED_NON_FUNCTIONAL, result);
}

TEST_F(CategorizationReasonTest, Unchanged_FatalErrors) {
    const char* result = getReasonForCategorization(
        static_cast<unsigned int>(ParsedDiffStatus::FATAL_ERRORS),
        static_cast<unsigned int>(UnParsedDiffStatus::UN_CHANGES),
        false
    );

    EXPECT_STREQ(CRITICAL_PARSING_ERRORS, result);
}

TEST_F(CategorizationReasonTest, Unchanged_UnsupportedUpdates_Compatible) {
    const char* result = getReasonForCategorization(
        static_cast<unsigned int>(ParsedDiffStatus::UNSUPPORTED_UPDATES),
        static_cast<unsigned int>(UnParsedDiffStatus::UN_CHANGES),
        true
    );

    EXPECT_STREQ(UNCHANGED_UNSUPPORTED_COMPATIBLE, result);
}

TEST_F(CategorizationReasonTest, Unchanged_UnsupportedUpdates_Incompatible) {
    const char* result = getReasonForCategorization(
        static_cast<unsigned int>(ParsedDiffStatus::UNSUPPORTED_UPDATES),
        static_cast<unsigned int>(UnParsedDiffStatus::UN_CHANGES),
        false
    );

    EXPECT_STREQ(UNCHANGED_UNSUPPORTED_INCOMPATIBLE, result);
}

TEST_F(CategorizationReasonTest, Unchanged_SupportedUpdates_Compatible) {
    const char* result = getReasonForCategorization(
        static_cast<unsigned int>(ParsedDiffStatus::SUPPORTED_UPDATES),
        static_cast<unsigned int>(UnParsedDiffStatus::UN_CHANGES),
        true
    );

    EXPECT_STREQ(UNCHANGED_SUPPORTED_COMPATIBLE, result);
}

TEST_F(CategorizationReasonTest, Unchanged_SupportedUpdates_Incompatible) {
    const char* result = getReasonForCategorization(
        static_cast<unsigned int>(ParsedDiffStatus::SUPPORTED_UPDATES),
        static_cast<unsigned int>(UnParsedDiffStatus::UN_CHANGES),
        false
    );

    EXPECT_STREQ(UNCHANGED_SUPPORTED_INCOMPATIBLE, result);
}

TEST_F(CategorizationReasonTest, Unchanged_CommentsUpdated) {
    const char* result = getReasonForCategorization(
        static_cast<unsigned int>(ParsedDiffStatus::COMMENTS_UPDATED),
        static_cast<unsigned int>(UnParsedDiffStatus::UN_CHANGES),
        false
    );

    EXPECT_STREQ(UNCHANGED_COMMENTS, result);
}

TEST_F(CategorizationReasonTest, Unchanged_NonFunctionalChanges) {
    const char* result = getReasonForCategorization(
        static_cast<unsigned int>(ParsedDiffStatus::NON_FUNCTIONAL_CHANGES),
        static_cast<unsigned int>(UnParsedDiffStatus::UN_CHANGES),
        false
    );

    EXPECT_STREQ(UNCHANGED_NON_FUNCTIONAL, result);
}

TEST_F(CategorizationReasonTest, InvalidParsedStatus_Changed) {
    const char* result = getReasonForCategorization(
        999,
        static_cast<unsigned int>(UnParsedDiffStatus::CHANGED),
        false
    );

    EXPECT_STREQ(ERROR_INVALID_PARSED_STATUS, result);
}

TEST_F(CategorizationReasonTest, InvalidParsedStatus_Unchanged) {
    const char* result = getReasonForCategorization(
        999,
        static_cast<unsigned int>(UnParsedDiffStatus::UN_CHANGES),
        false
    );

    EXPECT_STREQ(ERROR_INVALID_PARSED_STATUS, result);
}

TEST_F(CategorizationReasonTest, InvalidUnparsedStatus) {
    const char* result = getReasonForCategorization(
        static_cast<unsigned int>(ParsedDiffStatus::SUPPORTED_UPDATES),
        999,
        false
    );

    EXPECT_STREQ(ERROR_INVALID_UNPARSED_STATUS, result);
}
