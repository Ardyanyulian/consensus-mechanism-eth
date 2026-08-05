#include <iostream>
#include <vector>
#include <iomanip>
#include <cassert>

#include "../common/config/chain_config.h"
#include "../consensus/lmd_ghost.h"
#include "../consensus/casper_ffg.h"
#include "../common/rlp/encode_rlp.h"
#include "../common/rlp/decode_rlp.h"
#include "../common/crypto/keccak.h"

using namespace common;
using namespace common::consensus;
using namespace common::config;
using namespace common::crypto;

// Fungsi helper konversi hasil Keccak256 (32-byte std::vector) ke tipe types::Hash
types::Hash keccak_to_type_hash(const std::vector<uint8_t>& raw_bytes) {
    types::Hash h;
    for (size_t i = 0; i < h.bytes.size() && i < raw_bytes.size(); ++i) {
        h.bytes[i] = raw_bytes[i];
    }
    return h;
}

// Fungsi pembantu membuat hash menggunakan Keccak256
types::Hash create_keccak_hash(uint32_t val) {
    std::string input = "block_data_" + std::to_string(val);
    std::vector<uint8_t> hash_bytes = keccak256(input);
    return keccak_to_type_hash(hash_bytes);
}

void print_hash(const std::string& label, const types::Hash& hash) {
    std::cout << label << " 0x"
              << std::hex << std::setw(2) << std::setfill('0') << (int)hash.bytes[1]
              << std::setw(2) << std::setfill('0') << (int)hash.bytes[0]
              << std::dec << "...";
}

int main() {
    std::cout << "===============================================================\n";
    std::cout << "  TESTING & SIMULASI ETHEREUM CONSENSUS + RLP + KECCAK256      \n";
    std::cout << "===============================================================\n\n";

    // --- INTEGRASI CHAIN CONFIG ---
    ChainConfig config = ChainConfig::load_from_file("config.json");

    std::cout << "[CONFIG] Loaded Network Type: "
              << (config.network_type == NetworkType::MAINNET ? "MAINNET" : "DEVNET") << "\n";
    std::cout << "[CONFIG] Chain ID: " << config.chain_id << "\n";
    std::cout << "[CONFIG] Slot Duration: " << config.slot_duration_seconds << " seconds\n";
    std::cout << "[CONFIG] Slots per Epoch: " << config.slots_per_epoch << "\n\n";

    // --- TEST 1: SANITY CHECK RLP ENCODE / DECODE ---
    rlp::RLPItem original_item;
    original_item.is_list = false;
    original_item.data = {0x01, 0x02, 0x03, 0x04};

    auto encoded = rlp::rlp_encode(original_item);
    auto decoded = rlp::rlp_decode(encoded);
    assert(decoded.data == original_item.data);
    std::cout << "[SUCCESS] RLP Encoder/Decoder Sanity Check Passed!\n";

    // --- TEST 2: SANITY CHECK KECCAK256 HASHING ---
    std::string test_input = "";
    std::vector<uint8_t> keccak_out = keccak256(test_input);
    assert(keccak_out.size() == 32); // Keccak-256 harus selalu menghasilkan output 32-byte (256-bit)
    std::cout << "[SUCCESS] Keccak256 Cryptographic Engine Sanity Check Passed!\n\n";

    // --- TEST 3: SIMULASI CONSENSUS ---
    ValidatorMap validators;
    validators[1] = Validator{32, false};
    validators[2] = Validator{32, false};
    validators[3] = Validator{32, false};
    validators[4] = Validator{32, false};
    uint64_t total_active_stake = 128;

    BlockMap block_store;
    // Menggunakan Keccak256 untuk Genesis Root
    types::Hash genesis_hash = create_keccak_hash(0x0000);

    BeaconBlock genesis_block;
    genesis_block.slot = 0;
    genesis_block.parent_root = create_keccak_hash(0xFFFF);
    block_store[genesis_hash] = genesis_block;

    Checkpoint genesis_checkpoint{0, genesis_hash};
    CasperFFG casper(genesis_checkpoint);

    LatestMessages latest_messages;
    types::Hash current_head = genesis_hash;

    const uint64_t SLOTS_PER_EPOCH = config.slots_per_epoch;
    const uint64_t TOTAL_EPOCHS = 3;
    const uint64_t TOTAL_SLOTS = SLOTS_PER_EPOCH * TOTAL_EPOCHS;

    std::cout << "[INIT] Genesis Block dibuat dengan Keccak-256 Root. Epoch: 0, Slots per Epoch: " << SLOTS_PER_EPOCH << "\n";
    std::cout << "---------------------------------------------------------------\n";

    for (uint64_t slot = 1; slot <= TOTAL_SLOTS; ++slot) {
        types::Epoch current_epoch = slot / SLOTS_PER_EPOCH;

        // Membuat Block Hash autentik berbasis Keccak256
        types::Hash new_block_hash = create_keccak_hash(static_cast<uint32_t>(slot));
        BeaconBlock new_block;
        new_block.slot = slot;
        new_block.parent_root = current_head;
        block_store[new_block_hash] = new_block;

        std::vector<Attestation> slot_attestations;
        for (types::ValidatorIndex v_idx = 1; v_idx <= 4; ++v_idx) {
            slot_attestations.push_back({v_idx, new_block_hash, slot});
        }
        LMDGhost::process_attestations(slot_attestations, validators, block_store, latest_messages);
        current_head = LMDGhost::execute_lmd_ghost(genesis_hash, latest_messages, validators, block_store);

        Checkpoint source_cp = casper.get_justified_checkpoint();
        Checkpoint target_cp{current_epoch, current_head};

        std::vector<FFGVote> ffg_votes;
        for (types::ValidatorIndex v_idx = 1; v_idx <= 4; ++v_idx) {
            ffg_votes.push_back({v_idx, source_cp, target_cp});
        }
        casper.process_epoch_votes(current_epoch, ffg_votes, validators, total_active_stake);

        if (slot % 8 == 1 || slot % SLOTS_PER_EPOCH == 0) {
            std::cout << "[Slot " << std::setw(2) << slot << " | Epoch " << current_epoch << "] ";
            print_hash("Head: ", current_head);
            std::cout << std::endl;
        }

        if (slot % SLOTS_PER_EPOCH == 0) {
            std::cout << "\n===============================================================\n";
            std::cout << "  >>> BATAS EPOCH " << current_epoch << " TERCAPAI (Slot " << slot << ") <<<\n";
            std::cout << "===============================================================\n";

            casper.update_finality_state(current_epoch, current_head, total_active_stake);

            Checkpoint justified = casper.get_justified_checkpoint();
            Checkpoint finalized = casper.get_finalized_checkpoint();

            std::cout << "  [STATUS STATE KONSENSUS CASPER FFG]:\n";
            std::cout << "  * Justified Epoch : " << justified.epoch;
            print_hash(" (Block Root: ", justified.root);
            std::cout << ")\n";

            std::cout << "  * Finalized Epoch : " << finalized.epoch;
            print_hash(" (Block Root: ", finalized.root);
            std::cout << ")\n";
            std::cout << "---------------------------------------------------------------\n\n";
        }

        // PERUBAHAN: Delay waktu dihapus agar simulasi selesai secara langsung (instan)
    }

    std::cout << "===============================================================\n";
    std::cout << "  SIMULASI BERHASIL TANPA ERROR KONFLIK / COMPILATION FAIL     \n";
    std::cout << "===============================================================\n";

    return 0;
}
