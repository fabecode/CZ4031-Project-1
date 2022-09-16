#include <iostream>
#include <vector>
#include <string>
#include "memory.h"
#include "memory.cpp"

using namespace std;

int main(int argc, char **argv) {
    // creates a new instance of disk with specific blocksize
    disk *Disk = new disk(200);
    // add path to data here
    Disk->readDataFromFile(".\\datadata.tsv");
    Disk->deleteRecord("tt0000010");
    std::vector<block> *diskBlock = Disk->getBlock();
    Disk->reportStatistics();

    disk *Disk2 = new disk(500);
    // add path to data here
    Disk2->readDataFromFile(".\\data.tsv");

    //loop to insert add & key pair
//    int count = 0;
//    BPlusTree node;
//    for(block b:*diskBlock){
//        for(record r:b.records){
//            while(count < 10){
//                node.insert(&r,r.numVotes);
//                count++;
//            }
//        }
//    }
//
//    node.display(node.getRoot());

}
