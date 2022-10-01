#ifndef MEMORY_H
#define MEMORY_H

#include <vector>
#include <string>

struct record {
    char tconst[11];
    float averageRating;
    int numVotes;
};

// get records based on block index + offset from *records
struct blockAddress {
    int index;
    int offset;
};

class disk {
    private:
        void *memory;
        std::vector<std::pair<int, int>> freed;
        int currentIndex;
        int size;
        int blocksize;
        int numBlocks;
        int capacity;
        int timesAccessed;
        int totalBlocks;
        int toffset;
    public:
        disk(int capacity, int blocksize);

        ~disk();

        // prints all records in the database - remove later
        void printitems(blockAddress *baddr);

        // fetch record
        record *getRecord(blockAddress *addr);

        void *getBlock(int index);

        void printBlock(void *block, int index);

        // output the number of blocks used and the size of the database
        void reportStatistics();

        // return number of blocks 
        int getNumBlocks();

        // insert new record into empty block
        blockAddress *insertRecord(std::string tconst, float averageRating, int numVotes);

        void deleteRecord(blockAddress *bAddr);

        bool allocateBlock();

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

        int getSize() {
            return disk::size;
        }
};

#endif
