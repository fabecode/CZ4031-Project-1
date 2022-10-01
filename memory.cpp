#include "memory.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>

using namespace std;

//int getOffset(int size) {
//    return (int) (size - sizeof(int)) / 19;
//}

disk::disk(int capacity, int bsize) {
//    blocks = new block*[capacity/bsize];
//    for (int i=0; i<capacity/bsize; i++) {
//        block *tBlock = new block();
//        tBlock->records = operator new(bsize);
//        tBlock->size = (int) sizeof(int);
//    }
    disk::memory = operator new(capacity);
    memset(disk::memory, '\0', capacity);
    disk::currentBlock = memory;
    currentindex = 0;
    disk::currentSize = (int) sizeof(int);
    disk::capacity = capacity;
    disk::timesAccessed = 0;
    disk::size = 0;
    disk::blocksize = bsize;
    disk::numBlocks = 0;
    disk::toffset = 0;
}

disk::~disk() {
//    delete memory;
//    for (int i=0; i<disk::numBlocks; i++) {
//        delete disk::blocks[i];
//    }
}

// insert a new tuple into disk
//blockAddress *disk::insertRecord(std::string tconst, float averageRating, int numVotes) {
//    record tRecord = record();
//    tRecord.averageRating = averageRating;
//    tRecord.numVotes = numVotes;
//    tconst.copy(tRecord.tconst, tconst.length(), 0);
//    tRecord.tconst[tconst.length()] = '\0';
//
//    // there is a previously freed space, use it
//    if (!freed.empty()) {
//        // get the first freed pair
//        std::pair<int, void *> i = freed[0];
//        freed.erase(freed.begin());
//        // retrieve the block (simulate block access)
//        block *tBlock = disk::blocks[i.first];
//        int offset = getOffset(tBlock->size)*sizeof(record);
//        // add the record into the address pointed by the second item in the pair
//        std::memcpy((char *) i.second, &tRecord, sizeof(record));
//        // count the bytes used
//        tBlock->size += 19;
//        disk::size += 19;
//        blockAddress *bAddr = new blockAddress();
//        bAddr->index = i.first;
//        // bAddr->offset = offset;
//        bAddr->addr = i.second;
//        // return (char *)i.second;
//        return bAddr;
//    }
//
//    // try inserting to the last block if there is space in the block and on the disk
//    if (!disk::blocks.empty() && disk::blocks.back()->size + 19 < disk::blocksize) {
//        block *tblock = disk::blocks.back();
//        int offset = getOffset(tblock->size)*sizeof(record);
//        int index = disk::numBlocks - 1;
//        blockAddress *bAddr = new blockAddress();
//        // bAddr->offset = offset;
//        bAddr->index = index;
//        //if (index == 1084647014) {
//        //    cout << "here" << endl;
//        //}
//        char *addr = (char *)tblock->records+offset;
//        std::memcpy(addr, &tRecord, sizeof(record));
//        tblock->size += 19;
//        // count the bytes used
//        disk::size += 19;
//        bAddr->addr = addr;
//        return bAddr;
//        // return addr;
//    } else {
//        // No previously reclaimed space, and we cannot insert a new block
//        if (disk::freed.empty() && diskFull()) {
//            std::cout << "Unable to insert new records, disk has reached max capacity." << std::endl;
//            return nullptr;
//        }
//
//        // no previously freed space, and disk can accommodate another block
//        // all blocks are full (or disk empty), allocate a new block
//        block *tBlock = new block();
//        tBlock->records = operator new(disk::blocksize);
//        tBlock->size = (int) sizeof(int);
//        // set all to null
//        memset((char *)tBlock->records, '\0',disk::blocksize);
//        int offset = getOffset(tBlock->size)*sizeof(record);
//        int index = disk::numBlocks;
//        //if (index == 1084647014) {
//        //    cout << "here" << endl;
//        //}
//        blockAddress *bAddr = new blockAddress();
//        // bAddr->offset = offset;
//        bAddr->index = index;
//        char *addr = (char *)tBlock->records+offset;
//        bAddr->addr = addr;
//        std::memcpy(addr, &tRecord, sizeof(record));
//        tBlock->size += 19;
//
//        // update disk
//        disk::blocks.push_back(tBlock);
//        disk::size += tBlock->size;
//        disk::numBlocks += 1;
//        return bAddr;
//        // return addr;
//    }
//}

blockAddress *disk::insertRecord(std::string tconst, float averageRating, int numVotes) {
    blockAddress *bAddr = new blockAddress();
    if (diskFull()) {
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
        currentBlock = (char *)currentBlock + disk::blocksize;
    }

    bAddr->index = currentindex;
    bAddr->offset = toffset;
    char *addr = (char *)disk::memory+(currentindex*disk::blocksize+toffset);
    memcpy(addr, &tRecord, sizeof(record));
    currentSize += 19;
    disk::size += 19;
    toffset += sizeof(record);
    // cout << bAddr->index << " " << bAddr->offset << endl;

//    record tt;
//    char *taddr = (char *)currentBlock+bAddr->offset;
//    std::memcpy(&tt, taddr, sizeof(record));
//    cout << tt.tconst << " " << tt.averageRating << " " << tt.numVotes << endl;
    return bAddr;
}

// deletes a record from disk based on key (tconst)
//void disk::deleteRecord(std::string key) {
//    bool found = false;
//    bool emptyBlock = false;
//    int index = -1;
//
//    // for each block in disk
//    for (int i=0; i<disk::numBlocks; i++) {
//        block *currentBlock = disk::blocks[i];
//        // int items = (int) (currentBlock->size - sizeof(int)) / 19;
//        int items = disk::blocksize / 19;
//        // for each record in the block
//        for (int j=0; j<items; j++) {
//            record tRecord;
//            std::memcpy(&tRecord, (char *) currentBlock->records+sizeof(record)*j, sizeof(record));
//            // if the record matches
//            if (tRecord.tconst == key) {
//                // track the block and address where we can insert a new record
//                freed.push_back(make_pair(i, (char *) currentBlock->records+sizeof(record)*j));
//                // erase the old record
//                memset((char *)currentBlock->records+sizeof(record)*j, '\0',sizeof(record));
//                // update block size
//                currentBlock->size -= 19;
//                // update disk size
//                disk::size -= 19;
//                found = true;
//                emptyBlock = (currentBlock->size - sizeof(int) == 0);
//                if (emptyBlock) {
//                    index = i;
//                }
//            }
//        }
//        if (found) {
//            break;
//        }
//    }
//
//    // we remove the block if its empty from the disk
//    if (emptyBlock) {
//        // since the block is being removed, we no longer need to track if the block has space for a new record
//        disk::freed.erase(remove_if(disk::freed.begin(), disk::freed.end(), [&](std::pair<int, char*> p) {
//            return p.first == index;
//        }), disk::freed.end());
//        // remove the block
//        disk::blocks.erase(disk::blocks.begin() + index);
//        // update disk
//        disk::numBlocks -= 1;
//    }
//}

//void disk::deleteRecord(blockAddress *bAddr) {
//    // disk::increaseTimesAccessed();
//    block *tBlock = disk::blocks[bAddr->index];
//    freed.push_back(make_pair(bAddr->index, (char *)tBlock->records+bAddr->offset));
//    // erase the old record
//    memset((char *)tBlock->records+bAddr->offset, '\0',sizeof(record));
//    tBlock->size -= 19;
//    disk::size -= 19;
//    if (tBlock->size - sizeof(int) == 0) {
//        // since the block is being removed, we no longer need to track if the block has space for a new record
//        disk::freed.erase(remove_if(disk::freed.begin(), disk::freed.end(), [&](std::pair<int, char*> p) {
//            return p.first == bAddr->index;
//        }), disk::freed.end());
//        // remove the block
//        delete tBlock;
//        disk::blocks.erase(disk::blocks.begin() + bAddr->index);
//        // update disk
//        disk::numBlocks -= 1;
//    }
//}

// sanity check
void disk::printitems(blockAddress *baddr) {
    //if (i > disk::numBlocks) {
    //    return;
    //}
    char testBlock[sizeof(record)];
    memset(testBlock, '\0', sizeof(record));
    void *b;
    std::memcpy(b, (char *) memory+baddr->index, disk::blocksize);

    int items = disk::blocksize / 19;
    for (int j=0; j<items; j++) {
        record tt;

        std::memcpy(&tt, (char *)b+baddr->offset, sizeof(record));
        if (memcmp(&tt, testBlock, sizeof(record)) != 0){
            cout << tt.tconst << " " << tt.averageRating << " " << tt.numVotes << endl;
        }
    }
//    for (int i=0; i<disk::numBlocks; i++) {
//        block *b = disk::blocks[i];
//        cout << "---New block---" << endl;
//        int items = disk::blocksize / 19;
//        for (int j=0; j<items; j++) {
//            record tt;
//
//            std::memcpy(&tt, (char *) b->records+sizeof(record)*j, sizeof(record));
//            if (memcmp(&tt, testBlock, sizeof(record)) != 0){
//                cout << tt.tconst << " " << tt.averageRating << " " << tt.numVotes << endl;
//            }
//        }
//    }
}

//block *disk::getBlock(int index) {
//    // everytime we retrieve a block, increment the access time
//    disk::increaseTimesAccessed();
//    return (disk::blocks[index]);
//}

// output disk statistics
void disk::reportStatistics() {
    std::cout << "Number of Blocks used: " << disk::numBlocks << "." << std::endl;
    std::cout << "Database size (MB): " << ((disk::size*1.0) / 1000000) << "." << std::endl;
}

// return number of blocks
int disk::getNumBlocks() {
    return disk::numBlocks;
}


// check if we can add a new block without exceeding capacity
bool disk::diskFull() {
    return (disk::size + disk::blocksize) > disk::capacity;
}

record *disk::getRecord(blockAddress *addr) {
    // everytime we retrieve a block, increment the access time
    disk::increaseTimesAccessed();
    record *r = new record();
    memcpy(r, (char *)memory+addr->index*blocksize+addr->offset, sizeof(record));
    return r;
}