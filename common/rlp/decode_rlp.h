#ifndef COMMON_DECODE_RLP
#define COMMON_DECODE_RLP

#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept>


struct RLPItem {
    bool is_list;
    std::string data;
    std::vector<RLPItem> list;
};

// fungsi Penerima bytes dan mengesernya
inline size_t decode_length(const std::vector<uint8_t>& bytes, size_t& offset, uint8_t prefix_offset){
    size_t total = 0;
    size_t lenght_of_length = 0;

    if (prefix_offset == 0xB7){
        if (bytes[offset] <= 0xBF &&  bytes[offset] > 0xB7){
            lenght_of_length = bytes[offset] - prefix_offset;
            offset++;
        } else {
            throw std::runtime_error("RLP: Non match");
        }
    }

    if (prefix_offset == 0xF7){
        if (bytes[offset] <= 0xFF && bytes[offset] > 0xF7){
            lenght_of_length = bytes[offset] - prefix_offset;
            offset++;
        } else {
            throw std::runtime_error("RLP: Non match");
        }
    }

    for (size_t i = 0; i < lenght_of_length;++i){
        total = total << 8;
        total = total + bytes[offset++];
    }

    if (total <= 55){
        throw std::runtime_error("RLP: Non-minimal length encoding");
    }

    return total;
}

// fungsi Pembungkus Wrapper
RLPItem decode_internal(const std::vector<uint8_t>& bytes, size_t& offset);

// fungsi pembaca panjang data
RLPItem rlp_decode(const std::vector<uint8_t>& bytes);
#endif
