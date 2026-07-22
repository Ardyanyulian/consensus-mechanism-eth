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
RLPItem decode_internal(const std::vector<uint8_t>& bytes, size_t& offset){

    uint8_t prefix;
    size_t length;
    size_t list_bytes_len;
    size_t target_offset;


    RLPItem Item;
    // Seleksi dan offset harus lebih kecil dari ukuran bytes
    if (offset >= bytes.size()){
        throw std::runtime_error("THROW ERROR RLP: Unexpected end of data");
    }

    // Baca bita prefix
    prefix = bytes[offset];

    // Penentuan aturan RLP (berdasarkan spesifikasi RLP pada arsitektur etherum)
    // data berbentuk single byte
    if (prefix < 0x80){
        Item.is_list = false;
        Item.data.push_back(bytes[offset]);
        offset++;
        return Item;
    }

    // data berbentuk string pendek
    if (prefix >= 0x80 && prefix <= 0xB7){
        length = prefix - 0x80;
        offset++;
        if (offset + length > bytes.size()){
            throw std::runtime_error("THROW ERROR: RLP: String lenght out of bounds");
        }
        Item.is_list = false;

        Item.data.assign(bytes.begin() + offset,bytes.begin() + offset + length);

        offset += length;
        return Item;
    }

    // data berbentuk string panjang
    if (prefix > 0xB7 && prefix <= 0xBF){
        length = decode_length(bytes, offset, 0xB7);
        if (offset + length > bytes.size()){
            throw std::runtime_error("THROW ERROR: RLP long string length out of bounds");
        }
        Item.is_list = false;

        Item.data.assign(bytes.begin() + offset,bytes.begin() + offset + length);

        offset += length;
        return Item;
    }

    // data berbentuk list pendek
    if (prefix >= 0xC0 && prefix <= 0xF7) {
        list_bytes_len = prefix - 0xC0;
        offset++;
        target_offset = offset + list_bytes_len;

        if (target_offset > bytes.size()){
            throw std::runtime_error("THROW ERROR: RLP long string length out of bounds");
        }
        Item.is_list = true;
        while (offset < target_offset){
            Item.list.push_back(decode_internal(bytes,offset));
        }
        return Item;
    }

    // data berbentuk list panjang
    if (prefix > 0xF7 && prefix <= 0xFF){
        list_bytes_len = decode_length(bytes, offset, 0xF7);
        target_offset = offset + list_bytes_len;
        if (target_offset > bytes.size()){
            throw std::runtime_error("THROW ERROR: RLP long string length out of bounds");
        }
        Item.is_list = true;
        while (offset < target_offset){
            Item.list.push_back(decode_internal(bytes, offset));
        }
        return Item;
    }
}


// fungsi pembaca panjang data
RLPItem rlp_decode(const std::vector<uint8_t>& bytes){
    size_t offset = 0;
    RLPItem result = decode_internal(bytes, offset);
    if ( offset != bytes.size()){
        throw std::runtime_error("RLP: Trailing bytes detected after decoding");
    }
    return result;
}

#endif
