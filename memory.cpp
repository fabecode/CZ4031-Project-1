#include "memory.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>

using namespace std;

disk::disk(int capacity, int bsize) {
    disk::memory = operator new(capacity);
    memset(disk::memory, '\0', capacity);
    disk::currentindex = 0;
    disk::currentSize = (int) sizeof(int);
    disk::capacity = capacity;
    disk::timesAccessed = 0;
    disk::size = 0;
    disk::blocksize = bsize;
    disk::numBlocks = 0;
    disk::toffset = 0;
    disk::totalBlocks = capacity / bsize;
}

disk::~disk() {
}

blockAddress *disk::insertRecord(std::string tconst, float averageRating, int numVotes) {
    blockAddress *bAddr = new blockAddress();

    // disk is full and final block cant accept another record
    if (disk::numBlocks == disk::totalBlocks && disk::currentSize + 19 > disk::blocksize && freed.empty()) {
        return nullptr;
    }
    record tRecord = record();
    tRecord.averageRating = averageRating;
    tRecord.numVotes = numVotes;
    tconst.copy(tRecord.tconst, tconst.length(), 0);
    tRecord.tconst[tconst.length()] = '\0';
    if (currentSize + 19 > disk::blocksize) {
        currentindex += 1;
        disk::numBlocks += 1;
        currentSize = (int) sizeof(int);
        toffset = 0;
    }

    bAddr->index = currentindex;
    bAddr->offset = toffset;
    char *addr = (char *)disk::memory+(currentindex*disk::blocksize+toffset);
    memcpy(addr, &tRecord, sizeof(record));
    currentSize += 19;
    disk::size += 19;
    toffset += sizeof(record);
    return bAddr;
}

void disk::deleteRecord(blockAddress *bAddr) {
    disk::increaseTimesAccessed();
    freed.push_back(make_pair(bAddr->index, bAddr->offset));
    // erase the old record
    memset((char *) memory+bAddr->index*blocksize+bAddr->offset, '\0', sizeof(record));
    disk::size -= 19;
}

// sanity check
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

// output disk statistics
void disk::reportStatistics() {
    std::cout << "Number of Blocks used: " << disk::numBlocks << "." << std::endl;
    std::cout << "Database size (MB): " << ((disk::size*1.0) / 1000000) << "." << std::endl;
}

// return number of blocks
int disk::getNumBlocks() {
    return disk::numBlocks;
}

record *disk::getRecord(blockAddress *addr) {
    // everytime we retrieve a block, increment the access time
    disk::increaseTimesAccessed();
    //printitems(addr);
    record *r = new record();
    memcpy(r, (char *)memory+addr->index*blocksize+addr->offset, sizeof(record));
    return r;
}