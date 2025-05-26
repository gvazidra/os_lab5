#ifndef EMPLOY_H
#define EMPLOY_H
#include <set>

struct employee {
    int num;
    char name[10];
    double hours;
};

bool isUniqueID(const std::set<int>& existingIDs, int id);

#endif