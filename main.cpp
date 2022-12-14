#include <iostream>
#include <vector>
#include <string>
#include "memory.h"
#include "bplustree.h"

using namespace std;

double calculateAverage(disk *Disk, vector<void *> &items);
void conductExperiments(int blockSize);

int main(int argc, char **argv){
    conductExperiments(200);
    conductExperiments(500);
}

void conductExperiments(int blockSize) {  
    
    int blocksize = blockSize;
    ofstream out1("results_" + to_string(blocksize) + "B.txt");
    streambuf *coutbuf = std::cout.rdbuf();
    std::cout.rdbuf(out1.rdbuf());

    /*
    =============================================================
    Experiment 1:
    Store the data (which is about IMDb movives and described in Part 4) on the disk and report the following statistics:
      - The number of blocks;
      - The size of database;
    =============================================================
    */
    std::cout << "==================Experiment 1==================" << endl;
    disk *Disk = new disk(100000000, blocksize); // 100MB
    BPlusTree *bplustree = new BPlusTree(blocksize);
    std::ifstream infile("./data.tsv");
    if (!infile){
        std::cerr << "File failed to open.\n";
        exit(1);
    }
    std::string str;
    float avgRate;
    int nVotes, i=0;

    while (infile.peek() != EOF) {
        //consume the first line
        if (i==0) {
            i += 1;
            infile >> str >> str >> str;
            continue;
        }

        infile >> str >> avgRate >> nVotes;
        blockAddress *addr = Disk->insertRecord(str, avgRate, nVotes);
        bplustree->insert(addr, nVotes);
    }
    //Size of database = size of relational data + index
    Disk->reportStatistics();
    cout << "Size of database + index = " << (Disk->getSize() + bplustree->getNumNodes()*blocksize*1.0) / 1000000 << "MB."<< endl;

    /*
    =============================================================
    Experiment 2:
    Build a B+ tree on the attribute "averageRating" by inserting the records sequentially and report the following statistics:
    - The parameter n of the B + tree;
    - The number of nodes of the B + tree;
    - The height of the B + tree, i.e., the number of levels of the B + tree;
    - The root node and 1st child node (actual content);
    =============================================================
    */
    
    std::cout << "\n==================Experiment 2==================" << endl;
    

    std::cout << "Parameter n of the B+ tree    : " << bplustree->getMaxKeys() << endl;
    std::cout << "Number of nodes of the B+ tree: " << bplustree->getNumNodes() << endl; //numnodes to be updated
    std::cout << "Height of the B+ tree         : " << bplustree->getHeight(bplustree->getRoot()) << endl; //height to be updated
    std::cout << "Root Node of the B+ tree: \n";
    bplustree->displayNode(bplustree->getRoot()); //getRoot gets the root address
    std::cout << endl;
    std::cout << "1st Child Node: " << endl;
    bplustree->displayNode((Node *)bplustree->getRoot()->pointers[0]);

    /*
    =============================================================
    Experiment 3:
    - Retrieve movies with the "numVotes" equal to 500 and report the following statistics:
    - The number and the content of index nodes (first 5) the process accesses;
    - The number and the content of data blocks the process accesses;
    - The average of "averageRating's" of the records that are returned;
    =============================================================
    */


    std::cout << "\n==================Experiment 3==================" << endl;
    std::cout << "Retrieve movies with numVotes equal to 500..." << endl;
    vector<void *> temp = bplustree->searchRange(500, 500);

    // number of index nodes accessed is the height of bplus tree - 1
    std::cout << "Number of index nodes the process accesses: " << bplustree->getHeight(bplustree->getRoot()) - 1 << endl;
    std::cout << "Content of index nodes (1st 5 nodes): " << endl;
    vector<Node *> index = bplustree->getT();
    for (int i=0; i<index.size(); i++) {
        bplustree->displayNode(index[i]);
        if (i >= 5) {
            break;
        }
    }
    bplustree->removeT();
    std::cout << "\nContent of data blocks the process accesses (1st 5 blocks): " << endl;
    float average = calculateAverage(Disk, temp);
    std::cout << "Number of data blocks the process accesses: " << Disk->getTimesAccessed() << endl;
    Disk->resetTimesAccessed();
    std::cout << "Average of averageRating's of the records returned: " << average << endl;

    /*
    =============================================================
    Experiment 4:
    Retrieve those movies with the attribute ???numVotes??? from 30,000 to 40,000, both inclusively and report the following statistics:
        - The number and the content of index nodes (first 5) the process accesses;
        - The number and the content of data blocks the process accesses;
        - the average of ???averageRating???s??? of the records that are returned;
    =============================================================
    */
    
    std::cout << "\n==================Experiment 4==================" << endl;
    std::cout << "Retrieve movies with numVotes between 30,000 to 40,000..." << endl;
    vector<void *> temp2 = bplustree->searchRange(30000, 40000);

    std::cout << "Number of index nodes the process accesses: " << bplustree->getHeight(bplustree->getRoot()) - 1<< endl;
    std::cout << "Content of index nodes the process accesses (1st 5 nodes): " << endl;
    vector<Node *> indexT = bplustree->getT();
    for (int i=0; i<indexT.size(); i++) {
        bplustree->displayNode(indexT[i]);
        if (i >= 5) {
            break;
        }
    }
    //cout << avg2 / temp2.size() << endl;
    bplustree->removeT();
    std::cout << "\nContent of data blocks the process accesses (1st 5 blocks): " << endl;
    average = calculateAverage(Disk, temp2);
    std::cout << "Number of data blocks the process accesses: " << Disk->getTimesAccessed() << endl;
    Disk->resetTimesAccessed();
    std::cout << "Average of averageRating's of the records returned: " << average << endl;

    /*
    =============================================================
    Experiment 5:
    Delete movies with the attribute ???numVotes??? equal to 1000, update the B + tree accordingly,
    and report the following statistics:
    - The number of times that a node is deleted(or two nodes are merged) during the process of the updating the B + tree;
    - The number of nodes of the updated B + tree;
    - The height of the updated B + tree;
    - The root node and its 1st child nodes of the updated B + tree;
    =============================================================
    */

    std::cout << "\n==================Experiment 5==================" << endl;
    std::cout << "Delete movies with numVotes equal to 1000..." << endl;
    int numNodesBefore = bplustree->getNumNodes();
    bplustree->remove(1000);
    int numNodesAfter = bplustree->getNumNodes();
    int numNodesChange = numNodesBefore - numNodesAfter;
    std::cout << "The number of times that a node is deleted(or two nodes are merged): " << numNodesChange << endl;
    std::cout << "Number of nodes of updated B+ tree: " << bplustree->getNumNodes() << endl; //numnodes to be updated
    std::cout << "Height of updated B+ tree         : " << bplustree->getHeight(bplustree->getRoot()) << endl; //height to be updated
    std::cout << "Root Node of updated B+ tree:\n ";
    bplustree->displayNode(bplustree->getRoot());
    std::cout << endl;
    std::cout << "1st Child Node of updated B+ tree: " << endl;
    bplustree->displayNode((Node *)bplustree->getRoot()->pointers[0]);
    delete Disk;
}

double calculateAverage(disk *Disk, vector<void *> &items) {
    float avg = 0;
    int count = 0;
    bool printFlag = true;
    for (int i=0; i<items.size(); i++) { //iterate throughs the vector to print the records
        if (i >= 5) {
            printFlag = false;
        }
        record *r = Disk->getRecord((blockAddress *) items[i], printFlag);
        avg += r->averageRating;
        count += 1;
    }
    return avg / (count * 1.0);
}