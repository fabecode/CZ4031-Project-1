#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

struct record {
    string tconst;
    float averageRating;
    int numVotes;
};

struct block {
    vector<record> records;
    int size;

    block() : size(200), records() {}
};

struct disk {
    vector<block> blocks;
    int size;

    disk() : size(1000000), blocks() {}
};

int main() {
    disk Disk = disk();
    ifstream infile("..");
    if (!infile){
        cerr << "File failed to open.\n";
        exit(1);
    }

    string str;
    float avgRate;
    int nVotes;

    block tBlock = block();
    int i = 0;
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
        tRecord.tconst = str;

        if (tBlock.size >= 18) {
            tBlock.records.push_back(tRecord);
            tBlock.size -= 18;
        } else {
            Disk.blocks.push_back(tBlock);
            block tBlock = block();
            tBlock.records.push_back(tRecord);
            tBlock.size -= 18;
            Disk.size -= 200;
        }
    }

    cout << Disk.blocks[0].records[0].tconst << " " << Disk.blocks[0].records[0].averageRating << endl;
    cout << Disk.size << " " << Disk.blocks.size() << endl;
    cout << Disk.blocks[0].records.size();
}
