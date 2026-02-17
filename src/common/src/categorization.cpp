// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "categorization.hpp"
#include "diff_utils.hpp"
#include <llvm-14/llvm/Support/raw_ostream.h>

const char* serialize(OverAllStatus status){
    switch (status){
        case OverAllStatus::FATAL_ERRORS: return "FATAL_ERRORS";
        case OverAllStatus::BACKWARD_COMPATIABLE: return "BACKWARD_COMPATIBLE";
        case OverAllStatus::BACKWARD_INCOMPATIBLE: return "BACKWARD_INCOMPATIBLE";
        case OverAllStatus::UNSUPPORTED_UPDATES: return "UNSUPPORTED_UPDATES";
        case OverAllStatus::SUPPORTED_UPDATES: return "SUPPORTED_UPDATES";
        case OverAllStatus::COMMENTS_UPDATED: return "COMMENTS_UPDATED";
        case OverAllStatus::NON_FUNCTIONAL_CHANGES: return "NON_FUNCTIONAL_CHANGES";
        case OverAllStatus::IN_ACTIVE: return "IN_ACTIVE";
        case OverAllStatus::CATEGORIZATION_ERROR: return "CATEGORIZATION_ERROR";
        case OverAllStatus::NOT_CATEGORIZED: return "NOT_CATEGORIZED";
        default: return "UNKNOWN_STATUS";
    }
}

const char* getOverAllCategory(unsigned int parsedDiffStatus, unsigned int unParsedDiffStatus, bool compatibility){

    ParsedDiffStatus parsedStatus = static_cast<ParsedDiffStatus>(parsedDiffStatus);
    UnParsedDiffStatus unParsedStatus = static_cast<UnParsedDiffStatus>(unParsedDiffStatus);

    OverAllStatus overAllStatus = OverAllStatus::NOT_CATEGORIZED;

    switch (unParsedStatus) {
        case UnParsedDiffStatus::CHANGED:
            switch (parsedStatus) {
                case ParsedDiffStatus::FATAL_ERRORS:
                    overAllStatus = OverAllStatus::FATAL_ERRORS;
                    break;
                case ParsedDiffStatus::UNSUPPORTED_UPDATES:
                    overAllStatus = compatibility ? OverAllStatus::IN_ACTIVE : OverAllStatus::BACKWARD_INCOMPATIBLE;
                    break;
                case ParsedDiffStatus::SUPPORTED_UPDATES:
                    overAllStatus = compatibility ? OverAllStatus::IN_ACTIVE : OverAllStatus::BACKWARD_INCOMPATIBLE;
                    break;
                case ParsedDiffStatus::COMMENTS_UPDATED:
                    overAllStatus = compatibility ? OverAllStatus::IN_ACTIVE : OverAllStatus::CATEGORIZATION_ERROR;
                    break;
                case ParsedDiffStatus::NON_FUNCTIONAL_CHANGES:
                    overAllStatus = compatibility ? OverAllStatus::IN_ACTIVE : OverAllStatus::CATEGORIZATION_ERROR;
                    break;
                default:
                    overAllStatus = OverAllStatus::CATEGORIZATION_ERROR;
                    break;
            }
            break;
        case UnParsedDiffStatus::UN_CHANGES:
            switch (parsedStatus) {
                case ParsedDiffStatus::FATAL_ERRORS:
                    overAllStatus = OverAllStatus::FATAL_ERRORS;
                    break;
                case ParsedDiffStatus::UNSUPPORTED_UPDATES:
                    overAllStatus = compatibility ? OverAllStatus::UNSUPPORTED_UPDATES : OverAllStatus::BACKWARD_INCOMPATIBLE;
                    break;
                case ParsedDiffStatus::SUPPORTED_UPDATES:
                    overAllStatus = compatibility ? OverAllStatus::BACKWARD_COMPATIABLE : OverAllStatus::BACKWARD_INCOMPATIBLE;
                    break;
                case ParsedDiffStatus::COMMENTS_UPDATED:
                    overAllStatus = compatibility ? OverAllStatus::COMMENTS_UPDATED : OverAllStatus::CATEGORIZATION_ERROR;
                    break;
                case ParsedDiffStatus::NON_FUNCTIONAL_CHANGES:
                    overAllStatus = compatibility ? OverAllStatus::NON_FUNCTIONAL_CHANGES : OverAllStatus::CATEGORIZATION_ERROR;
                    break;
                default:
                    overAllStatus = OverAllStatus::CATEGORIZATION_ERROR;
                    break;
            }
            break;
        default:
            overAllStatus = OverAllStatus::CATEGORIZATION_ERROR;
            break;
    }

    return serialize(overAllStatus);

}


const char* getReasonForCategorization(unsigned int parsedDiffStatus, unsigned int unParsedDiffStatus, bool compatibility){
    ParsedDiffStatus parsedStatus = static_cast<ParsedDiffStatus>(parsedDiffStatus);
    UnParsedDiffStatus unParsedStatus = static_cast<UnParsedDiffStatus>(unParsedDiffStatus);

    switch (unParsedStatus) {
        case UnParsedDiffStatus::CHANGED:
            switch (parsedStatus) {
                case ParsedDiffStatus::FATAL_ERRORS:
                    return CRITICAL_PARSING_ERRORS;
                case ParsedDiffStatus::UNSUPPORTED_UPDATES:
                    return compatibility ? CHANGED_UNSUPPORTED_COMPATIBLE : CHANGED_UNSUPPORTED_INCOMPATIBLE;
                case ParsedDiffStatus::SUPPORTED_UPDATES:
                    return compatibility ? CHANGED_SUPPORTED_COMPATIBLE : CHANGED_SUPPORTED_INCOMPATIBLE;
                case ParsedDiffStatus::COMMENTS_UPDATED:
                    return CHANGED_COMMENTS;
                case ParsedDiffStatus::NON_FUNCTIONAL_CHANGES:
                    return CHANGED_NON_FUNCTIONAL;
                default:
                    return ERROR_INVALID_PARSED_STATUS;
            }
            break;
        case UnParsedDiffStatus::UN_CHANGES:
            switch (parsedStatus) {
                case ParsedDiffStatus::FATAL_ERRORS:
                    return CRITICAL_PARSING_ERRORS;
                case ParsedDiffStatus::UNSUPPORTED_UPDATES:
                    return compatibility ? UNCHANGED_UNSUPPORTED_COMPATIBLE : UNCHANGED_UNSUPPORTED_INCOMPATIBLE;
                case ParsedDiffStatus::SUPPORTED_UPDATES:
                    return compatibility ? UNCHANGED_SUPPORTED_COMPATIBLE : UNCHANGED_SUPPORTED_INCOMPATIBLE;
                case ParsedDiffStatus::COMMENTS_UPDATED:
                    return UNCHANGED_COMMENTS;
                case ParsedDiffStatus::NON_FUNCTIONAL_CHANGES:
                    return UNCHANGED_NON_FUNCTIONAL;
                default:
                    return ERROR_INVALID_PARSED_STATUS;
            }
            break;
        default:
            return ERROR_INVALID_UNPARSED_STATUS;
    }

    return ERROR_UNKNOWN;
}