#include "chain_config.h"
#include <fstream>
#include <iostream>
#include <string>
namespace common::config {

    ChainConfig ChainConfig::get_devnet_config(){
        ChainConfig config;
        config.chain_id = 1337;
        config.network_id = 1337;
        config.target_gas_limit = 30000000;
        config.genesis_time = 1700000000;
        config.slot_duration_seconds = 12;
        config.slots_per_epoch = 32;
        config.network_type = NetworkType::DEVNET;
        return config;
    }

    ChainConfig ChainConfig::get_mainnet_config(){
        ChainConfig config;
        config.chain_id = 1;
        config.network_id = 1;
        config.target_gas_limit = 30000000;
        config.genesis_time = 1600000000;
        config.slot_duration_seconds = 12;
        config.slots_per_epoch = 32;
        config.network_type = NetworkType::MAINNET;
        return config;
    }

    ChainConfig ChainConfig::load_from_file(const std::string &config_path){
        std::ifstream file(config_path);
        ChainConfig config = ChainConfig::get_devnet_config();
        std::string word;
        std::string type_value;
        // seleksi apakah file json ada atau tidak ada
        if (!file.is_open()) {
            std::cerr << "Failed to open config file: " << config_path << std::endl;
            return config;
        }


        while (file >> word){
            if (word == "\"chain_id\":" || word == "chain_id:"){
                file >> config.chain_id;
            }

            if (word == "\"network_id\":" || word == "network_id:"){
                file >> config.network_id;
            }

            if (word == "\"target_gas_limit\":" || word == "target_gas_limit:"){
                file >> config.target_gas_limit;
            }

            if (word == "\"genesis_time\":" || word == "genesis_time:"){
                file >> config.genesis_time;
            }

            if (word == "\"slot_duration_seconds\":" || word == "slot_duration_seconds:"){
                file >> config.slot_duration_seconds;
            }

            if (word == "\"slots_per_epoch\":" || word == "slots_per_epoch:"){
                file >> config.slots_per_epoch;
            }

            if (word == "\"network_type\":" || word == "network_type:"){
               file >> type_value;

               if (type_value == "\"MAINNET\"" || type_value == "MAINNET"){

                   config.network_type = NetworkType::MAINNET;

               } else if (type_value == "\"SEPOLIA\"" || type_value == "SEPOLIA") {

                   config.network_type = NetworkType::SEPOLIA;

               } else {

                   config.network_type = NetworkType::DEVNET;
               }
            }
        }

        file.close();
        return config;
    }
}
