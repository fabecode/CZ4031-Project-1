#include <iostream>
#include <vector>
#include <string>
#include "memory.h"

using namespace std;

int main(int argc, char **argv) {
    // creates a new instance of disk with specific blocksize
    disk *Disk = new disk(200);
    // add path to data here
    Disk->readDataFromFile(".\\data.tsv");
    //Disk->deleteRecord("tt0000001");

    std::vector<block> *diskBlock = Disk->getBlock();
    string temp = "tt0000002";
    temp.copy((*diskBlock)[0].records[0].tconst, 10, 0);

    Disk->printitems();

    disk *Disk2 = new disk(500);
    // add path to data here
    Disk2->readDataFromFile(".\\data.tsv");

    //Disk2->printitems();
}
