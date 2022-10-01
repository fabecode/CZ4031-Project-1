#ifndef MEMORY_H
#define MEMORY_H

#include <vector>
#include <string>

struct record {
    char tconst[11];
    float averageRating;
    int numVotes;
};

struct block {
    void *records;
    int size;
};

// get records based on block index + offset from *records
struct blockAddress {
    int index;
    int offset;
};

class disk {
    private:
        // std::vector<block *> blocks;
        //block **blocks;
        void *memory;
        std::vector<std::pair<int, char*>> freed;
        int size;
        int blocksize;
        int numBlocks;
        int capacity;
        int timesAccessed;
        void *currentBlock;
        int currentindex;
        int toffset;
        int currentSize;
    public:
        disk(int capacity, int blocksize);

        ~disk();

        // prints all records in the database - remove later
        void printitems(blockAddress *baddr);

        // simulates a block access
        block *getBlock(int index);

        // fetch record
        record *getRecord(blockAddress *addr);

        // output the number of blocks used and the size of the database
        void reportStatistics();

        // return number of blocks 
        int getNumBlocks();

        // insert new record into empty block
        blockAddress *insertRecord(std::string tconst, float averageRating, int numVotes);

        // deletes a record based on the key
        void deleteRecord(std::string key);

        //void deleteRecord(blockAddress *bAddr);

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

        float getSizeMB() {
            return ((disk::size*1.0) / 1000000);
        }
};

#endif
