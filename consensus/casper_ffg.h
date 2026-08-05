#ifndef CONSENSUS_CASPER_FFG_H
#define CONSENSUS_CASPER_FFG_H

#include "../common/types/consensus.h"
#include "../common/types/primitives.h"
#include "lmd_ghost.h"
#include <unordered_map>
#include <vector>

namespace common::consensus {

    struct Checkpoint {
        types::Epoch epoch{0};
        types::Hash root;

        bool operator==(const Checkpoint& other) const noexcept {
            return epoch == other.epoch && root == other.root;
        }
    };

    struct CheckpointHasher {
        std::size_t operator()(const Checkpoint& cp) const noexcept {
            std::size_t h1 = std::hash<uint64_t>{}(cp.epoch);
            std::size_t h2 = 0;
            std::memcpy(&h2, cp.root.bytes.data(), sizeof(std::size_t));
            return h1 ^ (h2 << 1);
        }
    };

    struct FFGVote {
        types::ValidatorIndex validator_index;
        Checkpoint source;
        Checkpoint target;
    };

    struct VoteTracker {
        std::vector<FFGVote> history;
    };

    class CasperFFG {
    public:
        explicit CasperFFG(Checkpoint genesis_checkpoint);

        void process_epoch_votes(
            types::Epoch current_epoch,
            const std::vector<FFGVote>& votes,
            const ValidatorMap& validators,
            uint64_t total_active_stake
        );

        void update_finality_state(types::Epoch current_epoch, const types::Hash& current_head_root, uint64_t total_active_stake);

        static bool is_slashable_vote(const FFGVote& new_vote, const std::vector<FFGVote>& history);

        [[nodiscard]] Checkpoint get_justified_checkpoint() const noexcept { return justified_checkpoint_; }
        [[nodiscard]] Checkpoint get_finalized_checkpoint() const noexcept { return finalized_checkpoint_; }

    private:
        Checkpoint justified_checkpoint_;
        Checkpoint finalized_checkpoint_;

        std::unordered_map<Checkpoint, std::unordered_map<Checkpoint, uint64_t, CheckpointHasher>, CheckpointHasher> vote_accumulations_;
        std::unordered_map<types::ValidatorIndex, VoteTracker> validator_vote_history_;
    };

} // namespace common::consensus

#endif // CONSENSUS_CASPER_FFG_H
