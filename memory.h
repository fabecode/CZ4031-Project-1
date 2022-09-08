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
        int numBlocks;
    public:
        disk(int blocksize);

        ~disk();

        // prints all records in the database
        void printitems();

        // create disk by reading from file
        void readDataFromFile(std::string filePath);

        // retrieves a single block
        block getBlock(int index);

        // output the number of blocks used and the size of the database
        void reportStatistics();

        // insert new record into empty block
        void insertRecords(std::string tconst, float averageRating, int numVotes);

        // deletes a record based on the key
        void deleteRecord(std::string key);
};

#endif
