#ifndef COMMON_RLP_TYPES_H
#define COMMON_RLP_TYPES_H

#include <vector>
#include <cstdint>

namespace common::rlp {

    struct RLPItem {
        bool is_list{false};
        std::vector<uint8_t> data;
        std::vector<RLPItem> list;
    };

} // namespace common::rlp

#endif // COMMON_RLP_TYPES_H
