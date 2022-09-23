#ifndef B_PLUS_TREE_H
#define B_PLUS_TREE_H

#include <array>
#include <vector>
#include "memory.h"

//Node in B+ Tree
class Node
{
private:
  // Variables
  void **pointers;                                  //to store Node address or record address
  float *keys;                                      //ptr to arr of key
  int numKeys;                                      //num of key
  bool isLeaf;                                      //true/false
  friend class BPlusTree;                           //access to private var

public:
  Node(int maxKeys); 
};

class BPlusTree
{
private:
  // Variables
  disk *disk;           //ptr to memory block
 //disk *index;          //ptr to memory index
  int maxKeys;          //max num of key
  int degree;           //degree of b+ tree
  int numNodes;         //num of node
  Node *root;           //ptr to main main root
  void *rootAddress;    //ptr to root address

  std::size_t nodeSize; 

  //Methods

  //void insert(float key, Node *cursorDiskAddress, Node *childDiskAddress);
  void insertInternal(float key, Node *cursorDiskAddress, Node *childDiskAddress);

  void displayTree(Node *cursor, std::vector<std::string> *s, int *level);
  
  void removeInternal(float key, Node *cursorDiskAddress, Node *childDiskAddress);

  Node *findParent(Node *, Node *);

public:
  // Methods

  // Constructor
  BPlusTree();

  //search
  //void search(float lowerBoundKey, float upperBoundKey);
  Node* search(int x, bool flag, bool print);

  //print B+ tree
  void display();

  //print node and content
  void displayNode(Node *node);

  //print block with its data
  void displayBlock(void *block);

  void insert(void* address,float key);

  void remove(float key);

  //get and set

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
};

#endif