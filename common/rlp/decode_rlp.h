#ifndef COMMON_DECODE_RLP_H
#define COMMON_DECODE_RLP_H

#include <cstdint>
#include <stdexcept>
#include <vector>
#include "rlp_types.h"

namespace common::rlp {

    inline size_t decode_length(const std::vector<uint8_t>& bytes, size_t& offset, uint8_t prefix_offset) {
        if (offset >= bytes.size()) {
            throw std::runtime_error("RLP: Length byte out of bounds");
        }

        uint8_t prefix = bytes[offset];
        size_t length_of_length = prefix - prefix_offset;
        offset++;

        if (offset + length_of_length > bytes.size()) {
            throw std::runtime_error("RLP: Length bytes extended out of bounds");
        }

        size_t total = 0;
        for (size_t i = 0; i < length_of_length; ++i) {
            total = (total << 8) | bytes[offset++];
        }

        if (total < 56) {
            throw std::runtime_error("RLP: Non-minimal length encoding");
        }

        return total;
    }

    inline RLPItem decode_internal(const std::vector<uint8_t>& bytes, size_t& offset) {
        if (offset >= bytes.size()) {
            throw std::runtime_error("RLP: Unexpected end of data");
        }

        RLPItem item;
        uint8_t prefix = bytes[offset];

        // 1. Single byte [0x00, 0x7f]
        if (prefix < 0x80) {
            item.is_list = false;
            item.data.push_back(bytes[offset++]);
            return item;
        }

        // 2. Short string (0 - 55 bytes long) [0x80, 0xb7]
        if (prefix <= 0xB7) {
            size_t length = prefix - 0x80;
            offset++;
            if (offset + length > bytes.size()) {
                throw std::runtime_error("RLP: Short string out of bounds");
            }
            if (length == 1 && bytes[offset] < 0x80) {
                throw std::runtime_error("RLP: Non-minimal single byte encoding");
            }
            item.is_list = false;
            item.data.assign(bytes.begin() + offset, bytes.begin() + offset + length);
            offset += length;
            return item;
        }

        // 3. Long string (> 55 bytes long) [0xb8, 0xbf]
        if (prefix <= 0xBF) {
            size_t length = decode_length(bytes, offset, 0xB7);
            if (offset + length > bytes.size()) {
                throw std::runtime_error("RLP: Long string data out of bounds");
            }
            item.is_list = false;
            item.data.assign(bytes.begin() + offset, bytes.begin() + offset + length);
            offset += length;
            return item;
        }

        // 4. Short list (0 - 55 bytes payload) [0xc0, 0xf7]
        if (prefix <= 0xF7) {
            size_t list_bytes_len = prefix - 0xC0;
            offset++;
            size_t target_offset = offset + list_bytes_len;
            if (target_offset > bytes.size()) {
                throw std::runtime_error("RLP: Short list out of bounds");
            }
            item.is_list = true;
            while (offset < target_offset) {
                item.list.push_back(decode_internal(bytes, offset));
            }
            return item;
        }

        // 5. Long list (> 55 bytes payload) [0xf8, 0xff]
        size_t list_bytes_len = decode_length(bytes, offset, 0xF7);
        size_t target_offset = offset + list_bytes_len;
        if (target_offset > bytes.size()) {
            throw std::runtime_error("RLP: Long list payload out of bounds");
        }
        item.is_list = true;
        while (offset < target_offset) {
            item.list.push_back(decode_internal(bytes, offset));
        }
        return item;
    }

    inline RLPItem rlp_decode(const std::vector<uint8_t>& bytes) {
        size_t offset = 0;
        RLPItem result = decode_internal(bytes, offset);
        if (offset != bytes.size()) {
            throw std::runtime_error("RLP: Trailing bytes detected");
        }
        return result;
    }

} // namespace common::rlp

#endif // COMMON_DECODE_RLP_H
