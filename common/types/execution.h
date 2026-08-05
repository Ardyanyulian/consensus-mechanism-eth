#ifndef COMMON_TYPES_EXECUTION_H
#define COMMON_TYPES_EXECUTION_H

#include <vector>
#include <cstdint>
#include "primitives.h"

namespace common::execution {

    struct Transaction {
        uint64_t nonce{0};
        common::types::Address to;
        common::types::uint256 value{};
        uint64_t gas_limit{0};
        std::vector<uint8_t> data;
        common::types::Signature signature;
    };

    struct Header {
        common::types::Hash parent_hash;
        common::types::Root state_root;
        common::types::Address beneficiary;
        uint64_t number{0};
        uint64_t timestamp{0};
        common::types::uint256 base_fee_per_gas{};
    };

    struct Account {
        uint64_t nonce{0};
        common::types::uint256 balance{};
        common::types::Root storage_root;
        common::types::Hash code_hash;
    };

} // namespace common::execution

#endif // COMMON_TYPES_EXECUTION_H
