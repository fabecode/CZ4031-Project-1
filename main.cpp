#include <iostream>
#include <vector>
#include <string>
#include "memory.h"
#include "memory.cpp"
#include "bplustree.h"
#include "bplustree.cpp"

using namespace std;

void print_bool(bool cond) {
    if (cond) {
        std::cout << "True\n";
    } else {
        std::cout << "False\n";
    }
}

int main(int argc, char **argv) {
    // creates a new instance of disk with specific blocksize
    disk *Disk = new disk(200);
    // add path to data here
    Disk->readDataFromFile(".\\data.tsv");
    //Disk->deleteRecord("tt0000001");

    std::vector<block> *diskBlock = Disk->getBlock();
    string temp = "tt0000002";
    temp.copy((*diskBlock)[0].records[0].tconst, 10, 0);

    //Disk->printitems();

    disk *Disk2 = new disk(500);
    // add path to data here
    Disk2->readDataFromFile(".\\data.tsv");

    //Disk2->printitems();

    //loop to insert add & key pair
    int count = 0;
    BPlusTree node;
    for(block b:*diskBlock){
        for(record r:b.records){
            while(count < 1){
                node.insert(&r,r.numVotes);
                count++;
            }
        }
    }

    node.display(node.getRoot());
    
}
