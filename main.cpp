#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include "memory.h"
#include "bplustree.h"

using namespace std;

int main(int argc, char **argv) {
    // reset buffer
    streambuf *coutbuf = std::cout.rdbuf();
    /*
    =============================================================
    Experiment 1:
    Store the data (which is about IMDb movives and described in Part 4) on the disk and report the following statistics:
      - The number of blocks;
      - The size of database;
    =============================================================
    */
    std::cout << "==================Experiment 1==================" << endl;
    disk *Disk = new disk(150000000, 200); // 150MB
    //disk *DiskIndex = new disk(350); // 350MB
    std::vector<block> *diskBlock = Disk->getBlock();
    Disk->readDataFromFile("./datadata.tsv");

    // Report statistics
    Disk->reportStatistics();
    //Disk->printitems();

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
    BPlusTree bplustree;

    for (block b: *diskBlock) {
        int items = (int) (b.size - sizeof(int)) / 19;
        for (int j=0; j<items; j++) {
            record R;
            std::memcpy(&R, (char *) b.records+sizeof(record)*j, sizeof(record));
            bplustree.insert(((char *)b.records+j*sizeof(record)), (float) R.numVotes);
        }
    }

    std::cout << "Parameter n of the B+ tree    : " << bplustree.getMaxKeys() << endl;
    std::cout << "Number of nodes of the B+ tree: " << bplustree.getNumNodes() << endl; //numnodes to be updated
    std::cout << "Height of the B+ tree         : " << bplustree.getHeight(bplustree.getRoot()) << endl; //height to be updated
    std::cout << "Root nodes and 1st child node : " << endl;

    cout << "\nRoot Node: " << endl;
    bplustree.displayNode(bplustree.getRoot()); //getRoot gets the root address
    cout << bplustree.getRoot() << endl;
    cout << "\n1st Child Node: " << endl;
    


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
    bplustree.search(500, true, false);

    std::cout << endl;
    std::cout << "Number of index nodes the process accesses: " << endl;
    std::cout << "Content of index nodes (first 5 nodes): " << endl;
    std::cout << "Average of averageRating's of the records returned: " << endl;

    /*
    =============================================================
    Experiment 4:
    Retrieve those movies with the attribute “numVotes” from 30,000 to 40,000, both inclusively and report the following statistics:
        - The number and the content of index nodes the process accesses;
        - The number and the content of data blocks the process accesses;
        - the average of “averageRating’s” of the records that are returned;
    =============================================================
    */
    std::cout << "\n==================Experiment 4==================" << endl;
    std::cout << "Number of index nodes the process accesses: " << endl;
    std::cout << "Content of index nodes the process accesses: " << endl;
    std::cout << "Number of data blocks the process accesses: " << endl;
    std::cout << "Content of data blocks the process accesses: " << endl;
    std::cout << "Average of averageRating's of the records that are returned: " << endl;


    /*
    =============================================================
    Experiment 5:
    Delete movies with the attribute “numVotes” equal to 1000, update the B + tree accordingly,
    and report the following statistics:
    - The number of times that a node is deleted(or two nodes are merged) during the process of the updating the B + tree;
    - The number nodes of the updated B + tree;
    - The height of the updated B + tree;
    - The root node and its 1st child nodes of the updated B + tree;
    =============================================================
    */

}
