#include "casper_ffg.h"

namespace common::consensus {

    CasperFFG::CasperFFG(Checkpoint genesis_checkpoint)
        : justified_checkpoint_(genesis_checkpoint),
          finalized_checkpoint_(genesis_checkpoint) {}

    bool CasperFFG::is_slashable_vote(const FFGVote& new_vote, const std::vector<FFGVote>& history) {
        for (const auto& past_vote : history) {
            // Rule 1: Double Voting
            if (new_vote.target.epoch == past_vote.target.epoch && !(new_vote.target == past_vote.target)) {
                return true;
            }

            // Rule 2: Surround Voting
            if (new_vote.source.epoch < past_vote.source.epoch && new_vote.target.epoch > past_vote.target.epoch) {
                return true;
            }

            if (past_vote.source.epoch < new_vote.source.epoch && past_vote.target.epoch > new_vote.target.epoch) {
                return true;
            }
        }
        return false;
    }

    void CasperFFG::process_epoch_votes(
        types::Epoch current_epoch,
        const std::vector<FFGVote>& votes,
        const ValidatorMap& validators,
        uint64_t total_active_stake
    ) {
        for (const auto& vote : votes) {
            auto val_it = validators.find(vote.validator_index);
            if (val_it == validators.end() || val_it->second.slashed) {
                continue;
            }

            auto& history = validator_vote_history_[vote.validator_index].history;
            if (is_slashable_vote(vote, history)) {
                continue;
            }

            history.push_back(vote);
            vote_accumulations_[vote.source][vote.target] += val_it->second.effective_balance;
        }
    }

    void CasperFFG::update_finality_state(types::Epoch current_epoch, const types::Hash& current_head_root, uint64_t total_active_stake) {
        Checkpoint candidate_target{current_epoch, current_head_root};
        uint64_t accumulated_stake = vote_accumulations_[justified_checkpoint_][candidate_target];

        // Aturan Supermajority Ethereum: accumulated >= 2/3 * total
        bool supermajority_reached = (accumulated_stake * 3 >= total_active_stake * 2);

        if (supermajority_reached) {
            Checkpoint previous_justified = justified_checkpoint_;
            justified_checkpoint_ = candidate_target;

            if (previous_justified.epoch + 1 == justified_checkpoint_.epoch) {
                finalized_checkpoint_ = previous_justified;
            }
        }
    }

} // namespace common::consensus
