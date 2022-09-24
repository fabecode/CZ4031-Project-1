#include "memory.h"
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cstring>

using namespace std;

int getOffset(int size) {
    return (int) (size - sizeof(int)) / 19;
}

disk::disk(int capacity, int bsize) {
    disk::capacity = capacity;
    disk::timesAccessed = 0;
    disk::size = 0;
    disk::blocksize = bsize;
    disk::numBlocks = 0;
}

disk::~disk() {

}

// reads all data from file and add to the disk
void disk::readDataFromFile(std::string filePath) {
    std::ifstream infile(filePath);
    if (!infile){
        std::cerr << "File failed to open.\n";
        exit(1);
    }

    // initialise a new block (either 200 or 500 - size of header)
    block tblock = {operator new (disk::blocksize - sizeof(int)), (int) sizeof(int) };
    std::string str;
    float avgRate;
    int nVotes, i=0;

    // reads all records from file
    while (infile.peek() != EOF) {
        //consume the first line
        if (i==0) {
            i += 1;
            infile >> str >> str >> str;
            continue;
        }
        infile >> str >> avgRate >> nVotes;
        record tRecord = record();
        tRecord.averageRating = avgRate;
        tRecord.numVotes = nVotes;
        str.copy(tRecord.tconst, str.length(), 0);
        tRecord.tconst[str.length()] = '\0';

        // if there is space for a new record
        if (tblock.size + 19 < disk::blocksize) {
            std::memcpy((char *) tblock.records+ getOffset(tblock.size)*sizeof(record), &tRecord, sizeof(record));
            tblock.size += 19;
        } else {
            // block is full, add it to disk
            disk::blocks.push_back(tblock);
            disk::numBlocks += 1;
            disk::size += disk::blocksize;

            // we reached the max capacity and cannot allocate a new block
            if (diskFull()) {
                std::cout << "Unable to insert more records, disk is full." << std::endl;
            }

            // disk has space for a new block, add the record to the block
            tblock = {operator new (disk::blocksize - sizeof(int)), sizeof(int) };
            std::memcpy((char *) tblock.records+getOffset(tblock.size)*sizeof(record), &tRecord, sizeof(record));
            tblock.size += 19;
        }
    }

    // add the last block into the disk if disk is not full and there are records in the last block
    if (tblock.size > sizeof(int) && !diskFull()) {
        disk::blocks.push_back(tblock);
        disk::numBlocks += 1;
        disk::size += disk::blocksize;
    }
}

// insert a new tuple into disk
void disk::insertRecord(std::string tconst, float averageRating, int numVotes) {
    record tRecord = record();
    tRecord.averageRating = averageRating;
    tRecord.numVotes = numVotes;
    tconst.copy(tRecord.tconst, tconst.length(), 0);
    tRecord.tconst[tconst.length()] = '\0';

    // try inserting to the last block if there is space
    if (!disk::blocks.empty() && disk::blocks.back().size + 19 < disk::blocksize) {
        block *tblock = &(disk::blocks.back());
        std::memcpy((char *) tblock->records+getOffset(tblock->size)*sizeof(record), &tRecord, sizeof(record));
        tblock->size += 19;
    } else {
        // No previously reclaimed space, and we cannot insert a new block
        if (diskFull() && disk::freed.empty()) {
            std::cout << "Unable to insert new record, disk has reached max capacity." << std::endl;
            return;
        }

        // there is a previously freed space, use it
        if (!freed.empty()) {
            // get the first freed pair
            std::pair<int, void *> i = freed[0];
            freed.erase(freed.begin());
            // retrieve the block
            block *tBlock = &(disk::blocks[i.first]);
            // add the record into the address pointed by the second item in the pair
            std::memcpy((char *) tBlock->records+getOffset(tBlock->size)*sizeof(record), &tRecord, sizeof(record));
        } else { // no previously freed space, and disk can accommodate another block
            // all blocks are full (or disk empty), allocate a new block
            block tBlock = {operator new (disk::blocksize - sizeof(int)), (int) sizeof(int) };
            std::memcpy((char *) tBlock.records+getOffset(tBlock.size)*sizeof(record), &tRecord, sizeof(record));
            tBlock.size += 19;

            // update disk
            disk::blocks.push_back(tBlock);
            disk::size += disk::blocksize;
            disk::numBlocks += 1;
        }
    }
}

// deletes a record from disk based on key (tconst)
void disk::deleteRecord(std::string key) {
    bool found = false;
    bool emptyBlock = false;
    int index = -1;

    // for each block in disk
    for (int i=0; i<disk::numBlocks; i++) {
        block *currentBlock = &(disk::blocks[i]);
        int items = (int) (currentBlock->size - sizeof(int)) / 19;
        // for each record in the block
        for (int j=0; j<items; j++) {
            record tRecord;
            std::memcpy(&tRecord, (char *) currentBlock->records+sizeof(record)*j, sizeof(record));
            // if the record matches
            if (tRecord.tconst == key) {
                // track the block and address where we can insert a new record
                freed.push_back(make_pair(i, (char *) currentBlock->records+sizeof(record)*j));
                // erase the old record
                memset((char *)currentBlock->records+sizeof(record)*j, '\0',sizeof(record));
                // update block size
                currentBlock->size -= 19;
                found = true;
                emptyBlock = (currentBlock->size - sizeof(int) == 0);
                if (emptyBlock) {
                    index = i;
                }
            }
        }
        if (found) {
            break;
        }
    }

    // we remove the block if its empty from the disk
    if (emptyBlock) {
        // since the block is being removed, we no longer need to track if the block has space for a new record
        disk::freed.erase(remove_if(disk::freed.begin(), disk::freed.end(), [&](std::pair<int, char*> p) {
            return p.first == index;
        }), disk::freed.end());

        // remove the block
        disk::blocks.erase(disk::blocks.begin() + index);
        // update disk
        disk::numBlocks -= 1;
        disk::size -= disk::blocksize;
    }
}

// sanity check
void disk::printitems() {
    char testBlock[sizeof(record)];
    memset(testBlock, '\0', sizeof(record));
    for (int i=0; i<disk::numBlocks; i++) {
        block b = disk::blocks[i];
        int items = (int) (b.size - sizeof(int)) / 19;
        for (int j=0; j<items; j++) {
            record tt;

            std::memcpy(&tt, (char *) b.records+sizeof(record)*j, sizeof(record));
            if (memcmp(&tt, testBlock, sizeof(record)) != 0){
                cout << tt.tconst << " " << tt.averageRating << " " << tt.numVotes << endl;
            }
        }
    }
}

// returns a reference to the disk, used to build the bp tree
std::vector<block> *disk::getBlock() {
    return (&(disk::blocks));
}

block disk::getBlock(int index) {
    return (disk::blocks[index]);
}

// output disk statistics
void disk::reportStatistics() {
    std::cout << "Number of Blocks used: " << disk::numBlocks << "." << std::endl;
    std::cout << "Database size (MB): " << ((disk::size*1.0) / 1000000) << "." << std::endl;
}

bool disk::diskFull() {
    return disk::capacity == disk::size;
}