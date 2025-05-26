#include "employ.h"
#include <set>

bool isUniqueID(const std::set<int>& existingIDs, int id) {
    return existingIDs.find(id) == existingIDs.end();
}