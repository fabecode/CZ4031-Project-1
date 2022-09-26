#ifndef B_PLUS_TREE_H
#define B_PLUS_TREE_H

#include <array>
#include <vector>
#include "memory.h"


using namespace std;

//Node in B+ Tree
class Node
{
public:
  // Variables
  void **pointers;                                  //to store Node address or record address
  float *keys;                                      //ptr to arr of key
  int numKeys;                                      //num of key
  bool isLeaf;                                      //true/false
  friend class BPlusTree;                           //access to private var
  Node(int maxKeys); 
};

class BPlusTree
{
public:
  // Variables
// disk *Disk;           //ptr to memory block
  int maxKeys;          //max num of key
  int degree;           //degree of b+ tree
  int numNodes;         //num of node
  Node *root;           //ptr to main main root
  void *rootAddress;    //ptr to root address

  size_t nodeSize; 
  vector<Node *> t;
  // Constructor
  BPlusTree();

  //Methods

  vector<Node *> getT() {
    return t;
  }

  void removeT() {
    t.clear();
  }
  //void insert(float key, Node *cursorDiskAddress, Node *childDiskAddress);
  void insertInternal(float key, Node *cursorDiskAddress, Node *childDiskAddress);

  void displayTree(Node *cursor, std::vector<std::string> *s, int *level);
  
  void removeInternal(float key, Node *cursorDiskAddress, Node *childDiskAddress);

  Node *findParent(Node *, Node *);

  int checkDuplicate(float key);

std::
  //search
  vector<void *> searchNumVotes(float lowerBoundKey, float upperBoundKey);

  Node* search(float x, bool flag, bool print);

  //print B+ tree
  void display();

  void displayNode(Node *node);

  void insert(void* address,float key);

  void remove(float key);

  //get and set
  int getHeight(Node* node);
  int getHeightData(Node* node);

  // Returns a pointer to the root of the B+ Tree.
  Node *getRoot()
  {
    return root;
  };

  // Returns num of degree 
  int getDegree()
  {
    return degree;
  }
  ;

  int getNumNodes()
  {
    return numNodes;
  }

  int getMaxKeys()
  {
    return maxKeys;
  }

 // int getNumVotes()
 // {
 //   return nVotes;
 // }
};

#endif