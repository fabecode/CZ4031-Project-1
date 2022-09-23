#include <iostream>
#include <vector>
#include <string>
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
    disk *Disk = new disk(150); // 150MB
    disk *DiskIndex = new disk(350); // 350MB
    std::vector<block> *diskBlock = Disk->getBlock();
    Disk->readDataFromFile("./data.tsv");

    // Report statistics
    Disk->reportStatistics();

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
    int count = 0;

    for (block b:*diskBlock){
        for(record r:b.records){
            while(count < 10){
                bplustree.insert(&r, (float)r.numVotes);
                count++;
            }
        }
    }

    std::cout << "Parameter n of the B+ tree    : " << bplustree.getMaxKeys() <<endl;
    std::cout << "Number of nodes of the B+ tree: " << bplustree.getNumNodes() <<endl; //numnodes to be updated
    std::cout << "Height of the B+ tree         : " << bplustree.getDegree() <<endl; //height to be updated
    std::cout << "Root nodes and 1st child node : " << endl;
    bplustree.display();


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
  std::cout <<"Retrieve movies with numVotes equal to 500..."<<endl;     
  std::cout << endl;
  std::cout <<"Number of index nodes the process accesses: " <<endl; 
  std::cout <<"Content of index nodes (first 5 nodes): " <<endl;
  std::cout <<"Average of averageRating's of the records returned: " <<endl;
  std::cout << "\nNo more records found for range " << 8.0 << " to " << 8.0 << endl;

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
  std::cout <<"Number of index nodes the process accesses: " <<endl; 
  std::cout <<"Content of index nodes the process accesses: "<<endl; 
  std::cout <<"Number of data blocks the process accesses: " <<endl;
  std::cout <<"Content of data blocks the process accesses: "<<endl; 
  std::cout <<"Average of averageRating's of the records that are returned: " <<endl; 

  

}
