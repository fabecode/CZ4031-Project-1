#include "memory.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>

using namespace std;

disk::disk(int capacity, int bsize) {
    // allocate memory
    disk::memory = operator new(capacity);
    // clear and set all to null
    memset(disk::memory, '\0', capacity);
    // always point to the start of the last allocated block <current index, current size>, <0, 0>
    disk::current = make_pair(0, 0);
    disk::capacity = capacity;
    // record how many time we access a block
    disk::timesAccessed = 0;
    // total size of the disk (note we assume 19 bytes for a record)
    disk::size = 0;
    disk::blocksize = bsize;
    // total number of blocks used
    disk::numBlocks = 0;
    // true offset off a block (note padding bytes are added which makes each record == 20 bytes)
    // we assume 19 bytes in all calculations
    disk::toffset = 0;
    disk::totalBlocks = capacity / bsize;
}

disk::~disk() {
}

blockAddress *disk::insertRecord(std::string tconst, float averageRating, int numVotes) {
    blockAddress *bAddr = new blockAddress();

    // disk is full, final block cant accept another record, and there is no entry in freed.
    if (disk::numBlocks == disk::totalBlocks && disk::current.second + 19 > disk::blocksize && freed.empty()) {
        return nullptr;
    }

    // create the new record;
    record tRecord = record();
    tRecord.averageRating = averageRating;
    tRecord.numVotes = numVotes;
    tconst.copy(tRecord.tconst, tconst.length(), 0);
    tRecord.tconst[tconst.length()] = '\0';

    // current last block cannot fit a new record, we "allocate" a new block, which is to move the currentIndex by 1
    if (current.second + 19 > disk::blocksize) {
        bool result = allocateBlock();
        // disk is full and cannot allocate new block
        if (!result) {
            return nullptr;
        }
    }

    // get the "address" of the block
    bAddr->index = current.first;
    bAddr->offset = toffset;
    // writes it to disk
    char *addr = (char *)disk::memory+(current.first*disk::blocksize+toffset);
    memcpy(addr, &tRecord, sizeof(record));
    // increment size of block used, and disk size
    current.second += 19;
    disk::size += 19;
    toffset += sizeof(record);
    return bAddr;
}

void disk::deleteRecord(blockAddress *bAddr) {
    disk::increaseTimesAccessed();
    freed.push_back(make_pair(bAddr->index, bAddr->offset));
    // erase the old record
    memset((char *) memory+bAddr->index*blocksize+bAddr->offset, '\0', sizeof(record));
    // decrement size
    disk::size -= 19;
}

void disk::printitems(blockAddress *baddr) {
    char testBlock[sizeof(record)];
    memset(testBlock, '\0', sizeof(record));
    void *b[disk::blocksize];
    std::memcpy(b, (char *) memory+baddr->index*disk::blocksize, disk::blocksize);

    int items = disk::blocksize / 19;
    for (int j=0; j<items; j++) {
        record tt;

        std::memcpy(&tt, (char *)b+j*sizeof(record), sizeof(record));
        if (memcmp(&tt, testBlock, sizeof(record)) != 0){
            cout << tt.tconst << " " << tt.averageRating << " " << tt.numVotes << endl;
        }
    }
}

void disk::printBlock(void *block, int index) {
    char testBlock[sizeof(record)];
    memset(testBlock, '\0', sizeof(record));
    int items = disk::blocksize / 19;
    cout << "------------------Printing contents of block "<< index <<"-------------------------" << endl;
    for (int j=0; j<items; j++) {
        record tt;

        std::memcpy(&tt, (char *)block+j*sizeof(record), sizeof(record));
        if (memcmp(&tt, testBlock, sizeof(record)) != 0){
            cout << tt.tconst << " " << tt.averageRating << " " << tt.numVotes << endl;
        }
    }
    cout << "----------------------------------------------------------------------------" << endl;
}

// output disk statistics
void disk::reportStatistics() {
    std::cout << "Number of Blocks used: " << disk::numBlocks << "." << std::endl;
    std::cout << "Database size (MB): " << ((disk::size*1.0) / 1000000) << "." << std::endl;
}

// return number of blocks
int disk::getNumBlocks() {
    return disk::numBlocks;
}

bool disk::allocateBlock() {
    if (disk::numBlocks == disk::totalBlocks) {
        return false;
    }
    disk::numBlocks += 1;
    current = make_pair(disk::numBlocks, 0);
    toffset = 0;
    return true;
}

record *disk::getRecord(blockAddress *addr) {
    // everytime we retrieve a block, increment the access time
    void *block = getBlock(addr->index);
    printBlock(block, addr->index);
    record *r = new record();
    memcpy(r, (char *)block+addr->offset, sizeof(record));
    return r;
}

void *disk::getBlock(int index) {
    disk::increaseTimesAccessed();
    void *block = operator new(disk::blocksize);
    std::memcpy(block, (char *) memory+index*disk::blocksize, disk::blocksize);
    return block;
}