#ifndef COMMON_ENCODE_RLP
#define COMMON_ENCODE_RLP

#include <cstdint>
#include <iterator>
#include <variant>
#include <vector>
#include <string>
#include <type_traits>


struct RLPItem {
    bool is_list;
    std::string data;
    std::vector<RLPItem> list;
};

// ini adalah fungsi untuk mengurutkan integral berdasarkan big endian
// dan ini memotong bit bit kosong didepan
// jadi hanya return bit bersih
template<typename datatype>
inline std::vector<uint8_t> big_endian_minimal(datatype data){
    static_assert(std::is_arithmetic<datatype>::value, "datatype must be an arithmetic type");

    // buat vector untuk menyimpan hasilnya
    std::vector<uint8_t> result;
    // buat variabel untuk menyimpan byte sementara
    // ini variabel penyimpan 1 bytes/8 bit/8 saklar 1 dan 0
    uint8_t byte;

    if (data == 0) {
        // jika data 0 return byte 0/vector 0
        return result;
    } else {
        bool found = false;
        // jumlah seluruh byte dari data - 1
        int i = sizeof(data) - 1;

        // perulangan untuk menggeser bit untuk menemukan
        // byte yang tidak kosong
        while (i >= 0) {
            // geser bit untuk mendapatkan byte ke i
            // lalu lakukan operasi AND dengan 0xFF untuk mendapatkan nilai byte
            byte = static_cast<uint8_t>((data >> (i* 8)) & 0xFF);

            // jika byte kososong dan belum ditemukan yang ada isinya
            // maka lanjutkan sampai ketemu isinya dan mulai masukkan
            // jika sudah ketemu byte yang ada isinya maka
            // masukkan semua bytes berikutnya sampai selesai meskipun itu byte 0
            if (byte != 0 || found) {
                // masukkan byte ke result
                result.push_back(byte);
                // tandai bahwa sudah ditemukan byte yang tidak kosong
                found = true;
            }
            i--;
        }
    }
    return result;
}

template<typename datatype>
inline std::vector<uint8_t> rlp_encode(const RLPItem& data){
    // penyimpanan hasil encoding dalam bentuk vector 1 byte
    std::vector<uint8_t> result;

    // cek apakah data adalah list atau bukan
    if (data.is_list){
        std::vector<uint8_t> payload;
        // jika data adalah list maka lakukan encoding untuk setiap item dalam list
        // dan masukkan hasil encoding ke result
        for (const auto& item : data.list) {
            // jika bentuk item itu list
            // maka ia di encode satu persatu dahulu baru dimasukkan ke result
            std::vector<uint8_t> encoded_item = rlp_encode<datatype>(item);
            // masukkan hasil encoding ke payload
            payload.insert(payload.end(), encoded_item.begin(), encoded_item.end());
        }

        // total seluruh panjang data yang sudah di encode
        size_t total_length = payload.size();
        // jika total panjang data kurang dari 56 byte maka masukkan prefix 0xC0 + total_length
        if (total_length < 56){
            // buat prefix 0xC0 + total_length
            result.push_back(static_cast<uint8_t>(0xC0 + total_length));
        } else {
            // masukkan total panjang data dalam bentuk big endian minimal
            std::vector<uint8_t> length_bytes = big_endian_minimal<size_t>(total_length);
            // masukkan prefix 0xF7 + jumlah byte yang digunakan untuk menyimpan panjang data
            result.push_back(static_cast<uint8_t>(0xF7 + length_bytes.size()));
            // masukkan byte panjang data ke result
            result.insert(result.end(), length_bytes.begin(), length_bytes.end());
        }
        // masukkan seluruh hasil payload tadi ke result
        // dalam format result =  prefix + length + data
        result.insert(result.end(), payload.begin(), payload.end());
    } else {

        // jika data bukan list maka lakukan encoding untuk data string
        const std::string& str_data = data.data;
        // masukkan panjang dari data string tersebut
        size_t str_length = str_data.length();

        // check apa lah data string itu hanya 1 byte atau kurang dari itu
        if ( str_length == 1 && static_cast<uint8_t>(str_data[0]) < 0x80){
            // kalau itu hanya 1 byte atau kurang maka
            // langsung push byte tersebut tanpa prefix
            // mengapa memakai static_cast<uint8_t>?
            // karena meungkinkan kita agar bisa memasukkannya
            // sebagai bentuk 8 bit/1 byte ke dalam vector result
            result.push_back(static_cast<uint8_t>(str_data[0]));
        // check jika data string itu kurang dari 56 byte
        } else if (str_length < 56) {
            // masukkan/ubah prefix + jumlah bytes untuk data string kurang dari 56 bytes
            result.push_back(static_cast<uint8_t>(0x80 + str_length));
            // masukkan data string ke result sebagai byte
            result.insert(result.end(), str_data.begin(), str_data.end());
        // maybe selain string
        } else {
            // masukkan hasil big endian dari panjang data dalam bentuk size_t kedalam
            // vector length_bytes untuk menyimpan panjang data dalam bentuk byte
            std::vector<uint8_t> length_bytes = big_endian_minimal<size_t>(str_length);

            // masukkan prefix + jumlah byte yang digunakan untuk menyimpan panjang data
            // mengapa memakai static_cast<uint8_t>?
            // karena meungkinkan kita agar bisa memasukkannya sebagai bentuk 8 bit/1 byte ke dalam vector result
            result.push_back(static_cast<uint8_t>(0xB7 + length_bytes.size()));
            // masukkan byte panjang data ke result
            result.insert(result.end(), length_bytes.begin(), length_bytes.end());
            // masukkan data string ke result sebagai byte
            result.insert(result.end(), str_data.begin(), str_data.end());
        }
    }
    return result;
}
#endif
