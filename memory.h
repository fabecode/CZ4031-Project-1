#ifndef MEMORY_H
#define MEMORY_H

#include <vector>
#include <string>

struct record {
    char tconst[11];
    float averageRating;
    int numVotes;
};

// calculate memory address of record based on formula
// disk::memory + blockAddress->index * disk::blocksize + blockAddress->offset
struct blockAddress {
    int index;
    int offset;
};

class disk {
    private:
        // block of memory allocated
        void *memory;
        // records freed space on disk
        std::vector<std::pair<int, int>> freed;
        // points to the start
        int currentIndex;
        // total size of disk
        int size;
        // blocksize either 200B or 500B based on experiment
        int blocksize;
        // how many blocks were used
        int numBlocks;
        // total size of disk (100MB)
        int capacity;
        // count the number of times we access a block
        int timesAccessed;
        // the maximum number of blocks disk can have, = capacity/blocksize
        int totalBlocks;
        // offset into the block
        int toffset;
    public:
        // disk constructor
        disk(int capacity, int blocksize);

        ~disk();

        // fetch record
        record *getRecord(blockAddress *addr, bool printFlag);

        // fetch a block on disk, simulates block level access
        void *getBlock(int index);

        void printBlock(void *block, int index);

        // output the number of blocks used and the size of the database
        void reportStatistics();

        // return number of blocks 
        int getNumBlocks();

        // insert new record into empty block
        blockAddress *insertRecord(std::string tconst, float averageRating, int numVotes);

        // deletes records on disk
        void deleteRecord(blockAddress *bAddr);

        // simulates "allocation" of new block
        bool allocateBlock();

        // helper functions
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
