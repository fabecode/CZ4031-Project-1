#include "memory.h"
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

using namespace std;

disk::disk(int bsize) {
    size = 0;
    blocksize = bsize;
    numBlocks = 0;
}

disk::~disk() {

}

void disk::readDataFromFile(std::string filePath) {
    std::ifstream infile(filePath);
    if (!infile){
        std::cerr << "File failed to open.\n";
        exit(1);
    }

    block tblock = block{std::vector<record>(), disk::blocksize};
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

        if (tblock.size >= 18) {
            tblock.records.push_back(tRecord);
            tblock.size -= 18;
        } else {
            blocks.push_back(tblock);
            tblock = block{ std::vector<record>(), disk::blocksize };

            tblock.records.push_back(tRecord);
            tblock.size -= 18;
            size += blocksize;
            disk::numBlocks += 1;
        }
    }

    // adding the final block if there are records
    if (!tblock.records.empty()) {
        blocks.push_back(tblock);
        size += disk::blocksize;
        disk::numBlocks += 1;
    }

    disk::reportStatistics();
}

void disk::insertRecords(std::string tconst, float averageRating, int numVotes) {
    record tRecord = record();
    tRecord.averageRating = averageRating;
    tRecord.numVotes = numVotes;
    tconst.copy(tRecord.tconst, 10, 0);

    // find a suitable block to insert record
    for (int i=0; i<blocks.size(); i++) {
        if (blocks[i].size >= 18) {
            blocks[i].records.push_back(tRecord);
            blocks[i].size -= 18;
            return;
        }
    }

    // all blocks are full, allocate a new block
    block tBlock = block{std::vector<record>(), disk::blocksize};
    blocks.push_back(tBlock);
    tBlock.records.push_back(tRecord);
    tBlock.size -= 18;
    size += disk::blocksize;
}

// deletes a record from disk based on key (tconst)
void disk::deleteRecord(std::string key) {
    // for each block in disk
    bool emptyBlock = false;
    for (int i=0; i<disk::numBlocks; i++) {
        block currentBlock = disk::blocks[i];

        // remove record if record.tconst == key
        currentBlock.records.erase(remove_if(currentBlock.records.begin(), currentBlock.records.end(),[&](record const r) {
           return r.tconst == key;
        }), currentBlock.records.end());

        currentBlock.size = (disk::blocksize - (currentBlock.records.size() * 18));
        disk::blocks[i] = currentBlock;
        if (currentBlock.records.empty()) {
            emptyBlock = true;
        }
    }

    if (emptyBlock) {
        disk::blocks.erase(remove_if(disk::blocks.begin(), disk::blocks.end(),[&](block const &b) {
            return b.records.empty();
        }), disk::blocks.end());
        disk::numBlocks -= 1;
        disk::size -= disk::blocksize;
    }
}

// sanity check
//void disk::printitems() {
//    for (int i=0; i<disk::blocks.size(); i++) {
//        std::vector<record> temp = blocks[i].records;
//        for (int j=0; j<temp.size(); j++) {
//            std::cout << temp[j].tconst << " " << temp[j].averageRating << " " << temp[j].numVotes << std::endl;
//        }
//    }
//}

// returns a reference to the disk, used to build the bp tree
std::vector<block> *disk::getBlock() {
    //return disk::blocks[index];
    return &(disk::blocks);
}

void disk::reportStatistics() {
    std::cout << "Number of Blocks used: " << disk::numBlocks << "." << std::endl;
    std::cout << "Database size (MB): " << ((disk::size*1.0) / 1048576) << "." << std::endl;
}

void disk::readDataFromFile2(std::string filePath) {
    std::ifstream infile(filePath);
    if (!infile){
        std::cerr << "File failed to open.\n";
        exit(1);
    }

    block tblock = block{std::vector<record>(), disk::blocksize};
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

        if (tblock.size >= 18) {
            tblock.records.push_back(tRecord);
            tblock.size -= 18;
        } else {
            blocks.push_back(tblock);
            tblock = block{ std::vector<record>(), disk::blocksize };

            tblock.records.push_back(tRecord);
            tblock.size -= 18;
            size += blocksize;
            disk::numBlocks += 1;
        }
    }

 

    disk::reportStatistics();
}