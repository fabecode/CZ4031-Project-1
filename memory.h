#ifndef MEMORY_H
#define MEMORY_H

#include <vector>
#include <string>

struct record {
    std::string tconst;
    float averageRating;
    int numVotes;
};

struct block {
    std::vector<record *> records;
    int size;

    block() : size(200), records() {}
};

struct disk {
    std::vector<block *> blocks;
    int size;

    disk() : size(0), blocks() {}
};

#endif
