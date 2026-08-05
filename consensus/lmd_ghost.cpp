#include "lmd_ghost.h"

namespace common::consensus {

    std::vector<types::Hash> LMDGhost::get_children(
        const types::Hash& parent_hash,
        const BlockMap& block_store
    ) {
        std::vector<types::Hash> children;
        for (const auto& [hash, block] : block_store) {
            if (block.parent_root == parent_hash) {
                children.push_back(hash);
            }
        }
        return children;
    }

    std::unordered_set<types::Hash, HashHasher, HashEqual> LMDGhost::get_all_descendants_inclusive(
        const types::Hash& target_root,
        const BlockMap& block_store
    ) {
        std::unordered_set<types::Hash, HashHasher, HashEqual> subtree;
        if (block_store.find(target_root) == block_store.end()) {
            return subtree;
        }

        std::vector<types::Hash> queue;
        queue.push_back(target_root);
        subtree.insert(target_root);

        size_t head = 0;
        while (head < queue.size()) {
            types::Hash current = queue[head++];
            std::vector<types::Hash> children = get_children(current, block_store);

            for (const auto& child : children) {
                if (subtree.find(child) == subtree.end()) {
                    subtree.insert(child);
                    queue.push_back(child);
                }
            }
        }
        return subtree;
    }

    uint64_t LMDGhost::calculate_subtree_weight(
        const types::Hash& target_root,
        const LatestMessages& latest_messages,
        const ValidatorMap& validators,
        const BlockMap& block_store
    ) {
        uint64_t total_weight = 0;
        auto subtree_set = get_all_descendants_inclusive(target_root, block_store);

        for (const auto& [v_idx, val_data] : validators) {
            if (val_data.slashed) continue;

            auto msg_it = latest_messages.find(v_idx);
            if (msg_it != latest_messages.end()) {
                const types::Hash& latest_vote = msg_it->second;
                if (subtree_set.find(latest_vote) != subtree_set.end()) {
                    total_weight += val_data.effective_balance;
                }
            }
        }
        return total_weight;
    }

    types::Hash LMDGhost::resolve_tie_breaker(const types::Hash& a, const types::Hash& b) {
        return (a < b) ? a : b;
    }

    types::Hash LMDGhost::execute_lmd_ghost(
        const types::Hash& root_block_hash,
        const LatestMessages& latest_messages,
        const ValidatorMap& validators,
        const BlockMap& block_store
    ) {
        types::Hash current_block = root_block_hash;

        while (true) {
            std::vector<types::Hash> children = get_children(current_block, block_store);

            if (children.empty()) {
                return current_block;
            }

            if (children.size() == 1) {
                current_block = children[0];
                continue;
            }

            types::Hash best_child = children[0];
            int64_t max_weight = -1;

            for (const auto& child : children) {
                uint64_t child_weight = calculate_subtree_weight(child, latest_messages, validators, block_store);

                if (static_cast<int64_t>(child_weight) > max_weight) {
                    max_weight = static_cast<int64_t>(child_weight);
                    best_child = child;
                } else if (static_cast<int64_t>(child_weight) == max_weight) {
                    best_child = resolve_tie_breaker(best_child, child);
                }
            }
            current_block = best_child;
        }
    }

    void LMDGhost::process_attestations(
        const std::vector<Attestation>& new_attestations,
        const ValidatorMap& validators,
        const BlockMap& block_store,
        LatestMessages& out_latest_messages
    ) {
        for (const auto& att : new_attestations) {
            bool is_valid_validator = (validators.find(att.validator_index) != validators.end());
            bool is_valid_block = (block_store.find(att.target_block_root) != block_store.end());

            if (is_valid_validator && is_valid_block) {
                out_latest_messages[att.validator_index] = att.target_block_root;
            }
        }
    }

} // namespace common::consensus
