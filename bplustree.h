#ifndef B_PLUS_TREE_H
#define B_PLUS_TREE_H

#include <array>
#include <vector>
#include "memory.h"


using namespace std;

//Node in B+ Tree
class Node {
    public:
        // Variables
        void **pointers;                                  //to store Node address or record address
        int *keys;                                      //ptr to arr of key
        int numKeys;                                      //num of key
        bool isLeaf;                                      //true/false
        Node(int maxKeys);
};

class OverflowNode {
    public:
        void **pointers;                                  //to store Node address or record address
        int numKeys;                                      //num of key
        bool isLeaf;                                      //true/false
        OverflowNode(int maxKeys);
};

class BPlusTree {
    public:
        // Variables
        int maxKeys;          //max num of key
        int numNodes;         //num of node
        Node *root;           //ptr to main main root
        //void *rootAddress;    //ptr to root address

        int overflowSize;
        vector<Node *> t;

        // Constructor
        BPlusTree(int blocksize);

        //Methods
        vector<Node *> getT() {
            return t;
        }

        void removeT() {
            t.clear();
        }

        //void insert(float key, Node *cursorDiskAddress, Node *childDiskAddress);
        void insertInternal(int key, Node *cursorDiskAddress, Node *childDiskAddress);

        void displayTree(Node *cursor, std::vector<std::string> *s, int *level);

        void removeInternal(int key, Node *cursorDiskAddress, Node *childDiskAddress);

        Node *findParent(Node *, Node *);

        int checkDuplicate(int key);

        //search
        std::vector<void *> searchNumVotes(int lowerBoundKey, int upperBoundKey);

        //print B+ tree
        void display();

        void displayNode(Node *node);

        void insert(void *address, int key);

        void remove(int key);

        //get and set
        int getHeight(Node *node);

        int getHeightData(Node *node);

        // Returns a pointer to the root of the B+ Tree.
        Node *getRoot() {
            return root;
        };

        int getNumNodes() {
            return numNodes;
        }

        int getMaxKeys() {
            return maxKeys;
        }

        // int getNumVotes()
        // {
        //   return nVotes;
        // }
};

#endif