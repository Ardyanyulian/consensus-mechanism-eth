#ifndef CONSENSUS_LMD_GHOST_H
#define CONSENSUS_LMD_GHOST_H

#include "../common/types/consensus.h"
#include "../common/types/primitives.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <cstring>

namespace common::consensus {

    struct HashHasher {
        std::size_t operator()(const types::Hash& hash) const noexcept {
            std::size_t seed = 0;
            std::memcpy(&seed, hash.bytes.data(), sizeof(std::size_t));
            return seed;
        }
    };

    struct HashEqual {
        bool operator()(const types::Hash& lhs, const types::Hash& rhs) const noexcept {
            return lhs.bytes == rhs.bytes;
        }
    };

    struct Attestation {
        types::ValidatorIndex validator_index;
        types::Hash target_block_root;
        types::Slot slot;
    };

    using BlockMap = std::unordered_map<types::Hash, BeaconBlock, HashHasher, HashEqual>;
    using ValidatorMap = std::unordered_map<types::ValidatorIndex, Validator>;
    using LatestMessages = std::unordered_map<types::ValidatorIndex, types::Hash>;

    class LMDGhost {
    public:
        static uint64_t calculate_subtree_weight(
            const types::Hash& target_root,
            const LatestMessages& latest_messages,
            const ValidatorMap& validators,
            const BlockMap& block_store
        );

        static types::Hash execute_lmd_ghost(
            const types::Hash& root_block_hash,
            const LatestMessages& latest_messages,
            const ValidatorMap& validators,
            const BlockMap& block_store
        );

        static void process_attestations(
            const std::vector<Attestation>& new_attestations,
            const ValidatorMap& validators,
            const BlockMap& block_store,
            LatestMessages& out_latest_messages
        );

    private:
        static std::vector<types::Hash> get_children(
            const types::Hash& parent_hash,
            const BlockMap& block_store
        );

        static std::unordered_set<types::Hash, HashHasher, HashEqual> get_all_descendants_inclusive(
            const types::Hash& target_root,
            const BlockMap& block_store
        );

        static types::Hash resolve_tie_breaker(
            const types::Hash& a,
            const types::Hash& b
        );
    };

} // namespace common::consensus

#endif // CONSENSUS_LMD_GHOST_H
