#include "memory.h"
#include <iostream>
#include <fstream>
#include <string>

disk::disk(int bsize) {
    size = 0;
    blocksize = bsize;
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
        tRecord.tconst[9] = '\0';

        if (tblock.size >= 18) {
            tblock.records.push_back(tRecord);
            tblock.size -= 18;
        } else {
            blocks.push_back(tblock);
            tblock = block{ std::vector<record>(), disk::blocksize };

            tblock.records.push_back(tRecord);
            tblock.size -= 18;
            size += disk::blocksize;
        }
    }

    // adding the final block if there are records
    if (tblock.size < disk::blocksize) {
        blocks.push_back(tblock);
        size += disk::blocksize;
    }

    std::cout << disk::blocksize << " " << "sanity" << std::endl;
}

void disk::insertRecords(std::string tconst, float averageRating, int numVotes) {
    record tRecord = record();
    tRecord.averageRating = averageRating;
    tRecord.numVotes = numVotes;
    tconst.copy(tRecord.tconst, 10, 0);
    tRecord.tconst[9] = '\0';

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

// sanity check
void disk::printitems() {
    std::cout << blocks[0].size << std::endl;
    std::cout << blocks.back().records.back().tconst << " " << blocks.back().records.back().averageRating << std::endl;
    std::cout << size << " " << blocks.size() << std::endl;
}