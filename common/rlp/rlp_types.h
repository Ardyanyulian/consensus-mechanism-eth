#ifndef COMMON_RLP_TYPES_H
#define COMMON_RLP_TYPES_H

#include <string>
#include <vector>

struct RLPItem {
    bool is_list;
    std::string data;
    std::vector<RLPItem> list;
};

#endif // COMMON_RLP_TYPES_H
