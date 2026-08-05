#ifndef COMMON_TYPES_CONSENSUS_H
#define COMMON_TYPES_CONSENSUS_H

#include "execution.h"
#include "primitives.h"

namespace common::consensus {

    struct Validator {
        types::Gwei effective_balance{0};
        bool slashed{false};

        Validator() = default;
        Validator(types::Gwei bal, bool sl) : effective_balance(bal), slashed(sl) {}
    };

    struct BeaconBlock {
        types::Slot slot{0};
        types::ValidatorIndex proposer_index{0};
        types::Hash parent_root;
        types::Hash execution_payload_hash;
    };

} // namespace common::consensus

#endif // COMMON_TYPES_CONSENSUS_H
