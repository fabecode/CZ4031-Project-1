#ifndef MEMORY_H
#define MEMORY_H

#include <vector>
#include <string>

struct record {
    char tconst[10];
    float averageRating;
    int numVotes;
};

struct block {
    std::vector<record> records;
    int size;
};

//struct disk {
//    std::vector<block> blocks;
//    int size;
//
//    disk() : size(0), blocks() {}
//};

class disk {
    private:
        std::vector<block> blocks;
        int size;
        int blocksize;
    public:
        disk(int blocksize);

        ~disk();

        void insertRecords(std::string tconst, float averageRating, int numVotes);

        void printitems();

        void readDataFromFile(std::string filePath);\
};

#endif
