#ifndef COMMON_CONFIG_CHAIN_CONFIG_H
#define COMMON_CONFIG_CHAIN_CONFIG_H

// include header yang digunakan saja
#include <cstdint>
#include <string>

namespace common::config {
        enum class NetworkType: uint8_t {
            MAINNET,
            SEPOLIA,
            DEVNET
        };

        struct ChainConfig {
            uint64_t chain_id = 1337;
            uint64_t network_id = 1337;
            uint64_t target_gas_limit = 30000000;

            uint64_t genesis_time = 1700000000;
            uint64_t slot_duration_seconds = 12;
            uint64_t slots_per_epoch = 32;
            NetworkType network_type = NetworkType::DEVNET;

            static ChainConfig load_from_file(const std::string& config_path);
            static ChainConfig get_devnet_config();
            static ChainConfig get_mainnet_config();
        };



} // namespace common::config

#endif // COMMON_CONFIG_CHAIN_CONFIG_H
