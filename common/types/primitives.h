#ifndef COMMON_TYPES_PRIMITIVES_H
#define COMMON_TYPES_PRIMITIVES_H

#include <array>
#include <cstdint>
#include <vector>

namespace common::types {

    struct Address {
        std::array<uint8_t, 20> bytes{};

        std::vector<uint8_t> to_vector() const {
            return std::vector<uint8_t>(bytes.begin(), bytes.end());
        }
    };

    struct Hash {
        std::array<uint8_t, 32> bytes{};

        bool operator==(const Hash& other) const noexcept {
            return bytes == other.bytes;
        }

        bool operator<(const Hash& other) const noexcept {
            return bytes < other.bytes;
        }
    };

    using Root = Hash;

    struct PublicKey {
        std::array<uint8_t, 48> bytes{};
    };

    struct Signature {
        std::array<uint8_t, 96> bytes{};
    };

    using Slot = uint64_t;
    using Epoch = uint64_t;
    using ValidatorIndex = uint64_t;
    using Gwei = uint64_t;
    using uint256 = std::array<uint64_t, 4>;

} // namespace common::types

#endif // COMMON_TYPES_PRIMITIVES_H
