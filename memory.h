#ifndef MEMORY_H
#define MEMORY_H

#include <vector>
#include <string>
#include <unordered_map>

struct record {
    char tconst[11];
    float averageRating;
    int numVotes;
};

struct block {
    void *records;
    //std::vector<record> records;
    int size;
};

class disk {
    private:
        std::vector<block> blocks;
        std::vector<std::pair<int, char *>> freed;
        int size;
        int blocksize;
        int numBlocks;
        int capacity;
        int timesAccessed;
    public:
        disk(int capacity, int blocksize);

        ~disk();

        // prints all records in the database - remove later
        void printitems();

        // create disk by reading from file
        void readDataFromFile(std::string filePath);

        // returns a reference to the disk, used to build the bp tree
        std::vector<block> *getBlock();

        // simulates a block access
        block getBlock(int index);

        // output the number of blocks used and the size of the database
        void reportStatistics();

        // insert new record into empty block
        void insertRecord(std::string tconst, float averageRating, int numVotes);

        // deletes a record based on the key
        void deleteRecord(std::string key);

        bool diskFull();

        int getTimesAccessed() {
            return disk::timesAccessed;
        }

        void increaseTimesAccessed() {
            disk::timesAccessed += 1;
        }

        void resetTimesAccessed() {
            disk::timesAccessed = 0;
        }
};

#endif
