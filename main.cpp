#include <iostream>
#include <vector>
#include <string>
#include <fstream>
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
    BPlusTree bplustree;
    std::ifstream infile("./datadata.tsv");
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
        void *addr = Disk->insertRecord(str, avgRate, nVotes);
        bplustree.insert((char *)addr, (float) nVotes);
    }
    //disk *DiskIndex = new disk(350); // 350MB
    //std::vector<block> *diskBlock = Disk->getBlock();
    //Disk->readDataFromFile("./datadata.tsv");

    // Creation of B+ Tree
    /*
    BPlusTree bplustree;

    for (block b: *diskBlock) {
        int items = (int) (b.size - sizeof(int)) / 19;
        for (int j=0; j<items; j++) {
            record R;
            std::memcpy(&R, (char *) b.records+sizeof(record)*j, sizeof(record));
            bplustree.insert(((char *)b.records+j*sizeof(record)), (float) R.numVotes);
        }
    }*/

    // Report statistics
    //Size of database = size of relational data + index
    Disk->reportStatistics();

    /*//Disk->printitems();

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
    

    std::cout << "Parameter n of the B+ tree    : " << bplustree.getMaxKeys() << endl;
    std::cout << "Number of nodes of the B+ tree: " << bplustree.getNumNodes() << endl; //numnodes to be updated
    std::cout << "Height of the B+ tree         : " << bplustree.getHeight(bplustree.getRoot()) << endl; //height to be updated
    std::cout << "Root Node: ";
    bplustree.displayNode(bplustree.getRoot()); //getRoot gets the root address
    std::cout << endl;
    std::cout << "\n1st Child Node: " << endl;
    bplustree.displayNode((Node *)bplustree.getRoot()->pointers[0]);
    bplustree.display();
    //Node *root = bplustree.getRoot();
    //Node *firstChild = (Node *) root->pointers[0];

    //for (int i=0; i<firstChild->numKeys; i++) {
    //    cout << firstChild->keys[i] << " ";
    //}


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
    vector<void *> temp = bplustree.searchNumVotes(651, 653);
    float avg = 0;
    cout << "temp size is " << temp.size() << endl;
    for (auto it : temp) { //iterate through the vector to print the records
        record R;
        std::memcpy(&R, (char *) it, sizeof(record));
        cout << R.tconst << " " << R.averageRating << " " << R.numVotes << endl;
        avg += R.averageRating;
    }

    std::cout << endl;
    // number of index nodes accessed is the height of bplus tree - 1
    std::cout << "Number of index nodes the process accesses: " << bplustree.getHeight(bplustree.getRoot()) - 1 << endl;
    std::cout << "Content of index nodes (first 5 nodes): " << endl;
    vector<Node *> index = bplustree.getT();    
    for (int i=0; i<index.size(); i++) {
        bplustree.displayNode(index[i]);
        if (i >= 5) {
            break;
        }
    }
    bplustree.removeT();
    std::cout << "Average of averageRating's of the records returned: " << endl;
    cout << avg / temp.size() << endl;

    /*
    =============================================================
    Experiment 4:
    Retrieve those movies with the attribute “numVotes” from 30,000 to 40,000, both inclusively and report the following statistics:
        - The number and the content of index nodes (first 5) the process accesses;
        - The number and the content of data blocks the process accesses;
        - the average of “averageRating’s” of the records that are returned;
    =============================================================
    */
    std::cout << "\n==================Experiment 4==================" << endl;
    std::cout << "Retrieve movies with numVotes between 30,000 to 40,000..." << endl;
    vector<void *> temp2 = bplustree.searchNumVotes(20, 1645);
    float avg2 = 0;
    for (auto it : temp2) { //iterate through the vector to print the records
        record R;
        std::memcpy(&R, (char *) it, sizeof(record));
        cout << R.tconst << " " << R.averageRating << " " << R.numVotes << endl;
        avg2 += R.averageRating;
    }

    std::cout << "Number of index nodes the process accesses: " << bplustree.getHeight(bplustree.getRoot()) - 1<< endl;
    std::cout << "Content of index nodes the process accesses: " << endl;
    vector<Node *> index = bplustree.getT();    
    for (int i=0; i<index.size(); i++) {
        bplustree.displayNode(index[i]);
        if (i >= 5) {
            break;
        }
    }
    cout << avg / temp.size() << endl;    
    bplustree.removeT();
    
    std::cout << "Number of data blocks the process accesses: " << endl;
    cout << avg / temp.size() << endl;
    
    
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

    bplustree.searchNumVotes(1000, 1000);

}
