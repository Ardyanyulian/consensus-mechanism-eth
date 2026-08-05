#ifndef COMMON_ENCODE_RLP_H
#define COMMON_ENCODE_RLP_H

#include <cstdint>
#include <type_traits>
#include <vector>
#include "rlp_types.h"

namespace common::rlp {

    template <typename T>
    inline std::vector<uint8_t> big_endian_minimal(T data) {
        static_assert(std::is_integral_v<T>, "T must be an integral type");
        if (data == 0) return {};

        std::vector<uint8_t> result;
        for (int i = sizeof(T) - 1; i >= 0; --i) {
            uint8_t byte = static_cast<uint8_t>((data >> (i * 8)) & 0xFF);
            if (!result.empty() || byte != 0) {
                result.push_back(byte);
            }
        }
        return result;
    }

    inline std::vector<uint8_t> rlp_encode(const RLPItem& data) {
        std::vector<uint8_t> result;

        if (data.is_list) {
            std::vector<uint8_t> payload;
            for (const auto& item : data.list) {
                std::vector<uint8_t> encoded_item = rlp_encode(item);
                payload.insert(payload.end(), encoded_item.begin(), encoded_item.end());
            }

            size_t total_length = payload.size();
            if (total_length < 56) {
                result.push_back(static_cast<uint8_t>(0xC0 + total_length));
            } else {
                std::vector<uint8_t> length_bytes = big_endian_minimal(total_length);
                result.push_back(static_cast<uint8_t>(0xF7 + length_bytes.size()));
                result.insert(result.end(), length_bytes.begin(), length_bytes.end());
            }
            result.insert(result.end(), payload.begin(), payload.end());
        } else {
            const auto& str_data = data.data;
            size_t str_length = str_data.size();

            if (str_length == 1 && str_data[0] < 0x80) {
                result.push_back(str_data[0]);
            } else if (str_length < 56) {
                result.push_back(static_cast<uint8_t>(0x80 + str_length));
                result.insert(result.end(), str_data.begin(), str_data.end());
            } else {
                std::vector<uint8_t> length_bytes = big_endian_minimal(str_length);
                result.push_back(static_cast<uint8_t>(0xB7 + length_bytes.size()));
                result.insert(result.end(), length_bytes.begin(), length_bytes.end());
                result.insert(result.end(), str_data.begin(), str_data.end());
            }
        }
        return result;
    }

} // namespace common::rlp

#endif // COMMON_ENCODE_RLP_H
