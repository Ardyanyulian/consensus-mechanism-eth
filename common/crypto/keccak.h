#ifndef COMMON_CRYPTO_KECCAK_H
#define COMMON_CRYPTO_KECCAK_H

#include <array>
#include <cstddef>
#include <vector>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <string>

namespace common::crypto {

        typedef uint64_t State[5][5];

        inline uint64_t ROTL64(uint64_t x, int n) {
            return (x << (n % 64)) | (x >> ((64 - (n % 64)) % 64));
        }

        inline void theta(State& A) {
            uint64_t C[5];

            for (int x = 0; x < 5; ++x) {
                C[x] = A[x][0] ^ A[x][1] ^ A[x][2] ^ A[x][3] ^ A[x][4];
            }

            for (int x = 0; x < 5; ++x) {
                uint64_t D = C[(x + 4) % 5] ^ ROTL64(C[(x + 1) % 5], 1);
                for (int y = 0; y < 5; ++y) {
                    A[x][y] ^= D;
                }
            }
        }

        inline void rho(State& A) {
            static const uint64_t rhoOffsets[5][5] = {
                {0,  36, 3,  41, 18},
                {1,  44, 10, 45, 2},
                {62, 6,  43, 15, 61},
                {28, 55, 25, 21, 56},
                {27, 20, 39, 8,  14}
            };

            for (int x = 0; x < 5; ++x) {
                for (int y = 0; y < 5; ++y) {
                    A[x][y] = ROTL64(A[x][y], rhoOffsets[x][y]);
                }
            }
        }

        inline void phi(State& A) {
            uint64_t temp[5][5];
            for (int x = 0; x < 5; ++x) {
                for (int y = 0; y < 5; ++y) {
                    temp[y][(2 * x + 3 * y) % 5] = A[x][y];
                }
            }
            for (int x = 0; x < 5; ++x) {
                for (int y = 0; y < 5; ++y) {
                    A[x][y] = temp[x][y];
                }
            }
        }

        inline void chi(State& A) {
            uint64_t temp[5][5];
            for (int x = 0; x < 5; ++x) {
                for (int y = 0; y < 5; ++y) {
                    temp[x][y] = A[x][y];
                }
            }

            for (int x = 0; x < 5; ++x) {
                for (int y = 0; y < 5; ++y) {
                    A[x][y] = temp[x][y] ^ ((~temp[(x + 1) % 5][y]) & temp[(x + 2) % 5][y]);
                }
            }
        }

        inline void iota(State& A, int round) {
            static const uint64_t roundConstants[24] = {
                0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL,
                0x8000000080008000ULL, 0x000000000000808bULL, 0x0000000080000001ULL,
                0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008aULL,
                0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
                0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL,
                0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL,
                0x000000000000800aULL, 0x800000008000000aULL, 0x8000000080008081ULL,
                0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL
            };

            A[0][0] ^= roundConstants[round];
        }

        inline void keccak_f1600(State& data) {
            for (int round = 0; round < 24; ++round) {
                theta(data);
                rho(data);
                phi(data);
                chi(data);
                iota(data, round);
            }
        }

        inline void absorb(State& data, const std::vector<uint8_t>& message) {
            const size_t rate = 136; // 1088 bit = 136 byte
            std::vector<uint8_t> p = message;

            // Logika Padding pad10*1 Keccak-256 Presisi
            size_t mod = p.size() % rate;
            p.push_back(0x01);
            while ((p.size() % rate) != (rate - 1)) {
                p.push_back(0x00);
            }
            p.push_back(0x80);

            // Jika sisa hanya 1 byte (misal pad10*1 di posisi yang sama), merge 0x01 | 0x80 = 0x81
            if (mod == rate - 1) {
                p.pop_back();
                p.pop_back();
                p.push_back(0x81);
            }

            // Memproses blok-blok 136-byte
            for (size_t offset = 0; offset < p.size(); offset += rate) {
                for (size_t i = 0; i < 17; ++i) { // 17 x 8 byte = 136 byte
                    uint64_t lane_value = 0;
                    for (size_t j = 0; j < 8; ++j) {
                        lane_value |= (static_cast<uint64_t>(p[offset + (i * 8) + j]) << (8 * j));
                    }
                    data[i % 5][i / 5] ^= lane_value;
                }

                keccak_f1600(data);
            }
        }

        inline std::vector<uint8_t> squeeze(State& data) {
            std::vector<uint8_t> hash_result;
            hash_result.reserve(32);

            for (int i = 0; i < 32; ++i) {
                int x = (i / 8) % 5;
                int y = (i / 8) / 5;
                uint8_t byte = (data[x][y] >> (8 * (i % 8))) & 0xFF;
                hash_result.push_back(byte);
            }

            return hash_result;
        }

        inline std::vector<uint8_t> keccak256(const std::vector<uint8_t>& input) {
            State state;
            std::memset(state, 0, sizeof(State));

            absorb(state, input);

            return squeeze(state);
        }

        inline std::vector<uint8_t> keccak256(const std::string &input) {
            std::vector<uint8_t> data(input.begin(), input.end());
            return keccak256(data);
        }

        template <typename datatype>
        std::vector<uint8_t> hash(const datatype& anydata) {
            const uint8_t *bytesPtr = reinterpret_cast<const uint8_t*>(&anydata);
            std::size_t bytesSize = sizeof(anydata);

            std::vector<uint8_t> raw_bytes(bytesPtr, bytesPtr + bytesSize);
            return keccak256(raw_bytes);
        }

        template <typename datatype>
        std::vector<uint8_t> hash(const std::vector<datatype>& anydata) {
            const uint8_t* bytesPtr = reinterpret_cast<const uint8_t*>(anydata.data());
            size_t bytesSize = anydata.size() * sizeof(datatype);

            std::vector<uint8_t> raw_bytes(bytesPtr, bytesPtr + bytesSize);
            return keccak256(raw_bytes);
        }

}

#endif // COMMON_CRYPTO_KECCAK_H
