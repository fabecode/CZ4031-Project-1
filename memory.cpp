#include "memory.h"
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

using namespace std;

disk::disk(int bsize) {
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

    // initialise block with empty record vector, and blocksize (either 200 or 500)
    block tblock = {std::vector<record>(), (uint16_t) disk::blocksize};
    //block{(std::__1::vector<record>)dynamic-init: <constructor-call>, (uint16_t)(uint16_t)(blocksize)}
    std::string str;
    float avgRate;
    int nVotes, i=0;

    while (infile.peek() != EOF) {
        //consume the first line
        if (i==0) {
            i++;
            infile >> str >> str >> str;
            continue;
        }
        infile >> str >> avgRate >> nVotes;
        record tRecord = record();
        tRecord.averageRating = avgRate;
        tRecord.numVotes = nVotes;
        str.copy(tRecord.tconst, 10, 0);

        // if current block is not full, block.size >= 0
        if (tblock.size - 18 >= 0) {
            tblock.records.push_back(tRecord);
            tblock.size = (uint16_t) (tblock.size - 18);
            // current block is full, we push into the disk and create a new block to insert the new record
        } else {
            // updating disk statistics
            disk::blocks.push_back(tblock);
            disk::size += disk::blocksize;
            disk::numBlocks += 1;

            // create and insert new block
            tblock = block { std::vector<record>(), (uint16_t) disk::blocksize };
            tblock.records.push_back(tRecord);
            tblock.size = (uint16_t) (tblock.size - 18);
        }
    }

    // adding the final block if there are records
    if (!tblock.records.empty()) {
        disk::blocks.push_back(tblock);
        disk::size += disk::blocksize;
        disk::numBlocks += 1;
    }

}

// insert a new tuple into disk
void disk::insertRecords(std::string tconst, float averageRating, int numVotes) {
    record tRecord = record();
    tRecord.averageRating = averageRating;
    tRecord.numVotes = numVotes;
    tconst.copy(tRecord.tconst, 10, 0);

    // find a suitable block to insert record
    for (int i=0; i<disk::numBlocks; i++) {
        if (blocks[i].size -18 >= 0) {
            blocks[i].records.push_back(tRecord);
            blocks[i].size = (uint16_t) (blocks[i].size - 18);
            return;
        }
    }

    // all blocks are full, allocate a new block
    block tBlock = {std::vector<record>(), (uint16_t) disk::blocksize};
    blocks.push_back(tBlock);
    tBlock.records.push_back(tRecord);
    tBlock.size = (uint16_t) (tBlock.size - 18);

    // update disk
    disk::size += disk::blocksize;
    disk::numBlocks += 1;
}

// deletes a record from disk based on key (tconst)
void disk::deleteRecord(std::string key) {
    // for each block in disk
    bool emptyBlock = false;

    for (int i=0; i<disk::numBlocks; i++) {
        block *currentBlock = &(disk::blocks[i]);

        // remove record if record.tconst == key
        currentBlock->records.erase(remove_if(currentBlock->records.begin(), currentBlock->records.end(), [&](record const &r) {
            return r.tconst == key;
        }), currentBlock->records.end());

        // update remaining size of block
        currentBlock->size = (uint16_t) (disk::blocksize - (currentBlock->records.size() * 18));
        if (currentBlock->records.empty()) {
            emptyBlock = true;
        }
    }

    // we remove the block if its empty from the disk
    if (emptyBlock) {
        disk::blocks.erase(remove_if(disk::blocks.begin(), disk::blocks.end(), [&](block const &b) {
            return b.records.empty();
        }), disk::blocks.end());
        disk::numBlocks -= 1;
        disk::size -= disk::blocksize;
    }
}

// sanity check
void disk::printitems() {
    for (int i=0; i<disk::blocks.size(); i++) {
        std::vector<record> temp = blocks[i].records;
        std::cout << blocks[i].size << std::endl;
        for (int j=0; j<temp.size(); j++) {
            std::cout << temp[j].tconst << " " << temp[j].averageRating << " " << temp[j].numVotes << std::endl;
        }
    }
}

// returns a reference to the disk, used to build the bp tree
std::vector<block> *disk::getBlock() {
    return &(disk::blocks);
}

// output disk statistics
void disk::reportStatistics() {
    std::cout << "Number of Blocks used: " << disk::numBlocks << "." << std::endl;
    std::cout << "Database size (MB): " << ((disk::size*1.0) / 1048576) << "." << std::endl;
}