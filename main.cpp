#include <iostream>
#include <vector>
#include <fstream>
#include "memory.h"

using namespace std;

int main(int argc, char **argv) {
    disk *Disk = new disk();
    // add path to data here
    ifstream infile("...");
    if (!infile){
        cerr << "File failed to open.\n";
        exit(1);
    }

    string str;
    float avgRate;
    int nVotes;

    block *tBlock = new block();
    int i = 0;
    while (infile.peek() != EOF) {
        //consume the first line
        if (i==0) {
            i++;
            infile >> str >> str >> str;
            continue;
        }
        infile >> str >> avgRate >> nVotes;

        record *tRecord = new record();
        tRecord->averageRating = avgRate;
        tRecord->numVotes = nVotes;
        tRecord->tconst = str;

        if (tBlock->size >= 18) {
            tBlock->records.push_back(tRecord);
            tBlock->size -= 18;
        } else {
            Disk->blocks.push_back(tBlock);
            tBlock = new block();
            tBlock->records.push_back(tRecord);
            tBlock->size -= 18;
            Disk->size += 200;
        }
    }

    // adding the final block
    Disk->blocks.push_back(tBlock);
    Disk->size += 200;

    cout << Disk->blocks[0]->size << endl;
    cout << Disk->blocks.back()->records.back()->tconst << " " << Disk->blocks.back()->records.back()->averageRating << endl;
    cout << Disk->size << " " << Disk->blocks.size() << endl;
}
