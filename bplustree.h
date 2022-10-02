#ifndef B_PLUS_TREE_H
#define B_PLUS_TREE_H

#include <array>
#include <vector>
#include "memory.h"
#include <fstream>

using namespace std;

//Node in B+ Tree
class Node {
    public:
        // Variables
        void **pointers;
        int *keys;
        int numKeys;
        bool isLeaf;
        Node(int maxKeys);
};

class OverflowNode {
    public:
        void **pointers;
        int numKeys;
        bool isLeaf;
        OverflowNode(int maxKeys);
};

class BPlusTree {
    public:
        // Variables
        int maxKeys;
        int numNodes;
        Node *root;

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
        void insertInternalNode(int key, Node *cursorDiskAddress, Node *childDiskAddress);

        void displayTree(Node *cursor, std::vector<std::string> *s, int *level);

        void removeInternalNode(int key, Node *cursorDiskAddress, Node *childDiskAddress);

        Node *findParent(Node *, Node *);

        int checkDuplicate(int key);

        //search
        std::vector<void *> searchRange(int lowerBoundKey, int upperBoundKey);

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