#include "bplustree.h"
#include "memory.h"

#include <iostream>
#include <cstring>
#include <array>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;

Node::Node(int maxKeys) {
    // Initialize array with key and ptr
    keys = new int[maxKeys];
    pointers = new void *[maxKeys + 1];
    // default nullptr and -1 key
    for (int i=0; i<maxKeys; i++) {
        pointers[i] = nullptr;
        keys[i] = -1;
    }
    pointers[maxKeys] = nullptr;
    isLeaf = false;
    numKeys = 0;
}

OverflowNode::OverflowNode(int maxKeys) {
    pointers = new void *[maxKeys];
    // default to nullptr
    for (int i=0; i<maxKeys; i++) {
        pointers[i] = nullptr;
    }
    isLeaf = false;
    numKeys = 0;
}

BPlusTree::BPlusTree(int blocksize) {
    // calculate number of pointers to store in overflow block
    overflowSize = (int) ((blocksize - sizeof(bool) - sizeof(int)) / sizeof(void *));
    // calculate number of key-pointers pair to store in node
    int left = (int) (blocksize - sizeof(bool) - sizeof(int) - sizeof(void *));
    maxKeys = 0;
    while (sizeof(int) + sizeof(void *) <= left) {
        maxKeys += 1;
        left -= (sizeof(int) + sizeof(void *));
    }
    // Initialize node
    root = nullptr;
    numNodes = 0;
}

// Insert a record into the B+ Tree. Key: Record's numOfVotes, Value: {blockAddress, offset}.
void BPlusTree::insert(void *address, int key) {
    // empty tree, create new root
    if (root == nullptr) {
        Node *newNode = new Node(maxKeys);
        newNode->keys[0] = key;
        newNode->isLeaf = true;
        newNode->numKeys += 1;
        OverflowNode *newOverflow = new OverflowNode(overflowSize);
        newOverflow->pointers[newOverflow->numKeys] = address;
        newOverflow->numKeys += 1;
        newOverflow->isLeaf = false;
        newNode->pointers[0] = newOverflow;
        root = newNode;
    } else {
        Node *cursor = root;
        Node *parent;
        int i;
        // searching the leaf node to insert record
        while (!cursor->isLeaf) {
            parent = cursor;
            for (i=0; i<cursor->numKeys; i++) {
                if (cursor->keys[i] > key) {
                    cursor = (Node *) cursor->pointers[i];
                    break;
                }

                if (i == cursor->numKeys - 1) {
                    cursor = (Node *) cursor->pointers[i+1];
                    break;
                }
            }
        }
        i = 0;
        // searching for position to insert record in leaf node
        while (key > cursor->keys[i] && i < cursor->numKeys)i++;
        // check if there is duplicates in the current leaf
        if (cursor->keys[i] == key) {
            OverflowNode *overflow = (OverflowNode *) cursor->pointers[i];
            OverflowNode *next;
            // follow linked list
            while (overflow) {
                next = overflow;
                overflow = (OverflowNode *) overflow->pointers[overflowSize-1];
            }
            overflow = next;
            if (overflow->numKeys + 1 < overflowSize) {
                overflow->pointers[overflow->numKeys] = address;
                overflow->numKeys += 1;
            } else {
                // create new overflow node as current is full
                OverflowNode *newOverflow = new OverflowNode(overflowSize);
                overflow->pointers[overflowSize - 1] = newOverflow;
                newOverflow->pointers[newOverflow->numKeys] = address;
                newOverflow->numKeys += 1;
            }
        } else {
            // key is not duplicated, create new overflownode
            OverflowNode *newOverflow = new OverflowNode(overflowSize);
            newOverflow->isLeaf = false;
            newOverflow->pointers[0] = address;
            newOverflow->numKeys += 1;
            // if leaf has space
            if (cursor->numKeys + 1 < maxKeys) {
                for (i=0; i<cursor->numKeys; i++) {
                    if (cursor->keys[i] > key) {
                        break;
                    }
                }
                // move keys and pointers back
                for (int j=cursor->numKeys; j>i; j--) {
                    cursor->pointers[j] = cursor->pointers[j-1];
                    cursor->keys[j] = cursor->keys[j-1];
                }
                // point it to the overflow node
                cursor->pointers[i] = newOverflow;
                cursor->keys[i] = key;
                cursor->numKeys += 1;
            } else {
                // update number of nodes
                BPlusTree::numNodes += 1;
                Node *tNode = new Node(maxKeys);
                tNode->isLeaf = true;
                // copy pointer to point to correct leaf node
                Node *next = (Node *)cursor->pointers[maxKeys];
                cursor->pointers[maxKeys] = tNode;
                tNode->pointers[maxKeys] = next;

                // create a temp vector to hold the maxKeys + 1 items
                std::vector<pair<int, void*>> tpointers;
                // push the new item in
                tpointers.push_back(make_pair(key, newOverflow));
                // move all the items in the node in
                for (i=0; i<cursor->numKeys; i++) {
                    tpointers.push_back(make_pair(cursor->keys[i], cursor->pointers[i]));
                    cursor->pointers[i] = nullptr;
                    cursor->keys[i] = -1;
                }
                // sort the temporary vector so that all the items are lined up correctly
                sort(tpointers.begin(), tpointers.end(), [&](pair<int, void*> p1, pair<int, void*> p2) {
                    return p1.first < p2.first;
                });

                // split the node in the middle
                cursor->numKeys = (maxKeys + 1) / 2;
                tNode->numKeys = (maxKeys + 1) - cursor->numKeys;
                int j=0;
                // assign them all
                for (i=0; i<=maxKeys; i++) {
                    if (i < cursor->numKeys) {
                        cursor->pointers[i] = tpointers[i].second;
                        cursor->keys[i] = tpointers[i].first;
                    } else {
                        tNode->pointers[j] = tpointers[i].second;
                        tNode->keys[j] = tpointers[i].first;
                        j += 1;
                    }
                }

                if (cursor == root) {
                    // created new root, update numNodes
                    BPlusTree::numNodes += 1;
                    Node *newRoot = new Node(maxKeys);
                    // We need to set the new root's key to be the left bound of the right child.
                    newRoot->keys[0] = tNode->keys[0];

                    // Point the new root's children as the existing node and the new node.
                    newRoot->pointers[0] = cursor;
                    newRoot->pointers[1] = tNode;

                    // Update new root's variables.
                    newRoot->isLeaf = false;
                    newRoot->numKeys = 1;

                    // Update the root node
                    root = newRoot;
                } else { 
                    // If we are not at the root, we need to insert a new parent in the middle levels of the tree.
                    insertInternalNode(tNode->keys[0], parent, tNode);
                }
            }
        }
    }
}

// Update parent node pointed and add internal node if needed.
// Takes in pointer of parent node, child node and the key to insert
void BPlusTree::insertInternalNode(int key, Node *cursor, Node *child) {
    if (cursor->numKeys < maxKeys) {

        // Searching the location to put child node in parent node
        int i = 0;
        if (isnan(key)) {
            while (child->keys[child->numKeys] > cursor->keys[i])i++;
        } else {
            while (key > cursor->keys[i] && i < cursor->numKeys)i++;
        }

        // Use bubble swap to insert the key to new child
        for (int j = cursor->numKeys; j > i; j--) {
            cursor->keys[j] = cursor->keys[j - 1];
        }

        // Shift pointers to right by 1
        for (int j = cursor->numKeys + 1; j > i + 1; j--) {
            cursor->pointers[j] = cursor->pointers[j - 1];
        }

        // Add new keys in parent node
        cursor->keys[i] = key;
        cursor->numKeys++;
        cursor->pointers[i + 1] = child;
    } else {

        // Parent node doesn't have space, make new internal node
        Node *newInternal = new Node(maxKeys);
        numNodes++;

        auto tempKeyList = new int[maxKeys + 1];
        auto tempPointerList = new Node *[maxKeys + 2];

        // Copy keys into temp key list.
        for (int i = 0; i < maxKeys; i++) {
            tempKeyList[i] = cursor->keys[i];
        }

        // Copy pointers into temp pointer list.
        for (int i = 0; i < maxKeys + 1; i++) {
            tempPointerList[i] = (Node *) cursor->pointers[i];
        }

        // Searching for position to insert key
        int i = 0;
        if (isnan(key)) {
            while (child->keys[child->numKeys] > tempKeyList[i])i++;
        } else {
            while (key > tempKeyList[i] && i < maxKeys)i++;
        }

        // Shift element with higher key backward
        int j;
        for (int j = maxKeys; j > i; j--) {
            tempKeyList[j] = tempKeyList[j - 1];
        }

        // Insert new key 
        tempKeyList[i] = key;

        // Shift corresponding pointers backward
        for (int j = maxKeys + 1; j > i + 1; j--) {
            tempPointerList[j] = tempPointerList[j - 1];
        }

        // Insert this pointer to child
        tempPointerList[i + 1] = child;
        newInternal->isLeaf = false; 

        cursor->numKeys = (maxKeys + 1) / 2;
        newInternal->numKeys = maxKeys - (maxKeys + 1) / 2;

        for (int i = 0; i < cursor->numKeys; i++) {
            cursor->keys[i] = tempKeyList[i];
        }

        // Insert new keys into the new internal node.
        for (i = 0, j = cursor->numKeys + 1; i < newInternal->numKeys; i++, j++) {
            newInternal->keys[i] = tempKeyList[j];
        }

        for (i = 0; i < cursor->numKeys + 1; i++) {
            cursor->pointers[i] = tempPointerList[i];
        }

        // Insert pointers into the new internal node.
        for (i = 0, j = cursor->numKeys + 1; i < newInternal->numKeys + 1; i++, j++) {
            newInternal->pointers[i] = tempPointerList[j];
        }
        for (int i = cursor->numKeys; i < maxKeys; i++) {
            cursor->keys[i] = float();
        }

        for (int i = cursor->numKeys + 2; i < maxKeys + 1; i++) {
            cursor->pointers[i] = nullptr;
        }


        // If current cursor is the root of the tree, we need to create a new root.
        if (cursor == root) {
            Node *newRoot = new Node(maxKeys);
            // Update newRoot to hold the children.
            newRoot->keys[0] = tempKeyList[cursor->numKeys];

            newRoot->pointers[0] = cursor;
            newRoot->pointers[1] = newInternal;

            newRoot->isLeaf = false;
            newRoot->numKeys = 1;

            root = newRoot;

        }
        else {
            // Otherwise, parent is internal, so we need to split and make a new parent internally again.
            // This is done recursively if needed.
            insertInternalNode(tempKeyList[cursor->numKeys], findParent(root, cursor), newInternal);
        }
    }
}

Node *BPlusTree::findParent(Node *cursor, Node *child) {
    Node *parent;
    if (cursor->isLeaf || ((Node *) cursor->pointers[0])->isLeaf) {
        return NULL;
    }
    for (int i = 0; i < cursor->numKeys + 1; i++) {
        if (cursor->pointers[i] == child) {
            parent = cursor;
            return parent;
        } else {
            parent = findParent((Node *) cursor->pointers[i], child);
            if (parent != NULL)
                return parent;
        }
    }
    return parent;
}

std::vector<void *> BPlusTree::searchRange(int lowerBoundKey, int upperBoundKey) {
    //search logic
    if (root == NULL) {
        throw std::logic_error("Tree is empty");
    } else {
        Node *cursor = root;
        // travel to the leaf node that might contain the key
        while (cursor->isLeaf == false) {
            t.push_back(cursor);
            for (int i = 0; i < cursor->numKeys; i++) {
                if (isnan(cursor->keys[i]) || lowerBoundKey < cursor->keys[i]) {
                    //Keep looping till data hit upper bound and stop
                    cursor = (Node *) cursor->pointers[i];
                    break;
                }
                if (i == cursor->numKeys - 1) {

                    cursor = (Node *) cursor->pointers[i + 1];
                    break;
                }
            }
        }

        // search if the key exists
        std::vector<void *> records;
        int i = 0;
        int x = 0;
        float key = cursor->keys[0];
        cout << endl;
        // move cursor to correct lower bound
        for( i = 0; i < cursor->numKeys; i++){
            key = cursor->keys[i];
            if(key >= lowerBoundKey) break;
        }
        // or the key that >= lowerbound might be at the next node
        if( i == cursor->numKeys ){
            cout << cursor->numKeys << endl;
            cursor = (Node *) cursor->pointers[maxKeys];
            if(!cursor)return records;
            if( cursor->keys[0] >= lowerBoundKey )i = 0;
            else{
                // not record within the range
                return records;
            }
        }

        // start traversing through all the leaf till we meet upper bound
        x=i;
        while( cursor->keys[x]<=upperBoundKey){
            //Handle overflownode contents
            OverflowNode *ofnode = (OverflowNode *)cursor->pointers[x];
            while (ofnode) {
                for(int j = 0; j < ofnode->numKeys ; j++){
                    records.push_back(ofnode->pointers[j]);
                }
                ofnode = (OverflowNode *) ofnode->pointers[overflowSize-1];
            }

            x++;
            if(x == cursor->numKeys){
                if(cursor->pointers[maxKeys]){
                    cursor = (Node *) cursor->pointers[maxKeys];
                    x = 0;
                }
                else break;
            }

        }
        return records;
    }
}

// Display a node and its contents in the B+ Tree.
void BPlusTree::displayNode(Node *node) {
    // Print out all contents in the node as such |pointer|key|pointer|
    int i = 0;
    std::cout << "|";

    for (int i = 0; i < node->numKeys; i++) {
        //   std::cout << node->pointers[i] << " | ";
        if (!isnan(node->keys[i])) {
            std::cout << node->keys[i] << " | ";
        }
    }
    std::cout << endl;
}

int BPlusTree::getHeight(Node *node) {
    if (node->isLeaf == false) {
        return BPlusTree::getHeight((Node *) node->pointers[0]) + 1;
    } else if (node->isLeaf == true) {
        return 1;
    } else {
        return 0;
    }
}

void BPlusTree::displayTree(Node *cursor, std::vector<std::string> *s, int *level) {
    if (cursor->isLeaf) {
        // cout << "Enter leaf\n";
        string item;
        item.append("|");
        for (int i = 0; i < cursor->numKeys; i++) {
            item.append(to_string((int) cursor->keys[i]));
            item.append("|");
        }
        item.append(" ");
        *level = 1;
        if (s->size() < *level) {
            s->push_back(item);
        } else {
            s->at((*level) - 1).append(item);
        }
        return;
    }

    string item;
    for (int i = 0; i < cursor->numKeys + 1; i++) {
        displayTree((Node *) cursor->pointers[i], s, level);

    }
    item.append("|");
    for (int i = 0; i < cursor->numKeys; i++) {
        if (isnan(cursor->keys[i])) item.append("NaN");
        else item.append(to_string((int) cursor->keys[i]));
        item.append("|");
    }
    item.append(" ");
    (*level) += 1;
    s->at(*level - 2).append("    ");
    if (s->size() < *level) {
        s->push_back(item);
    } else {
        s->at(*level - 1).append(item);
    }
    return;

}

void BPlusTree::display() {
    Node *root = getRoot();
    vector<string> s;
    int level = 0;

    Node *cursor = root;

    displayTree(cursor, &s, &level);

    for (int i = s.size() - 1; i > 0; i--) {
        int spaceCount = (s[0].size() - s[i].size()) / 2;
        for (spaceCount; spaceCount > 0; spaceCount--) {
            cout << " ";
        }
        cout << s[i] << "\n";

    }
    cout << s[0];

}

void BPlusTree::remove(int key) {

    if (root == NULL) {
        cout << "Tree empty\n";
    } else {
        Node *cursor = root;
        Node *parent;
        int leftSibling, rightSibling;

        //travel to the leaf node that might contain the key
        while (cursor->isLeaf == false) {
            int i;
            for (i=0; i < cursor->numKeys; i++){
                if(key < cursor->keys[i])break;
            }
            parent = cursor;
            leftSibling = i - 1;
            rightSibling = i + 1;
            cursor = (Node *) cursor->pointers[i];

        }
        //searching for the key in the leaf node
        bool found = false;
        int pos;
        for (pos = 0; pos < cursor->numKeys; pos++) {
            if (cursor->keys[pos] == key) {
                found = true;
                break;
            }
        }

        // can't find the key
        if (!found) {
            cout << "Not found\n";
            return;
        }

        //Found the key, delete overflow nodes
        OverflowNode *curr = (OverflowNode*) cursor->pointers[pos];
        free(curr);

        //shifting keys forward to overwrite it
        for (int i = pos; i < cursor->numKeys; i++) {
            cursor->keys[i] = cursor->keys[i + 1];
            cursor->pointers[i] = cursor->pointers[i + 1];
        }
        cursor->numKeys--;
        cursor->pointers[cursor->numKeys] = cursor->pointers[cursor->numKeys + 1];
        cursor->pointers[cursor->numKeys + 1] = nullptr;

        //If the key is in internal node, change to the next key
        int i = 0;
        while(parent->keys[i] <= key && i < parent->numKeys){
            if(parent->keys[i] == key)parent->keys[i] = cursor->keys[pos];
            i++;
        }

        if (cursor == root) {
            if (cursor->numKeys == 0) {
                cout << "Tree is empty!\n";
                delete[] cursor->keys;
                delete[] cursor->pointers;
                delete cursor;
                root = nullptr;
            }
            return;
        }

        if (cursor->numKeys >= (maxKeys + 1) / 2) {
            return;
        }
        if (leftSibling >= 0) {
            Node *leftNode = (Node *) parent->pointers[leftSibling];

            //Check if able to borrow from left
            if (leftNode->numKeys >= (maxKeys + 1) / 2 + 1) {

                //Shift the last pointers first
                cursor->pointers[cursor->numKeys + 1] = cursor->pointers[cursor->numKeys];

                //Shift both keys and pointers backward
                for (int i = cursor->numKeys; i > 0; i--) {
                    cursor->keys[i] = cursor->keys[i - 1];
                    cursor->pointers[i] = cursor->pointers[i - 1];
                }

                //move the left sibling's last key & ptr to current node
                cursor->numKeys++;
                cursor->keys[0] = leftNode->keys[leftNode->numKeys - 1];
                cursor->pointers[0] = leftNode->pointers[leftNode->numKeys - 1];

                // update left node
                leftNode->numKeys--;
                leftNode->pointers[leftNode->numKeys] = cursor;
                leftNode->pointers[leftNode->numKeys + 1] = nullptr;
                parent->keys[leftSibling] = cursor->keys[0];
                return;
            }
        }
        if (rightSibling <= parent->numKeys) {
            Node *rightNode = (Node *) parent->pointers[rightSibling];

            // check if able to borrow from right
            if (rightNode->numKeys >= (maxKeys + 1) / 2 + 1) {
                cursor->numKeys++;
                cursor->pointers[cursor->numKeys] = cursor->pointers[cursor->numKeys - 1];
                cursor->keys[cursor->numKeys - 1] = rightNode->keys[0];
                cursor->pointers[cursor->numKeys - 1] = rightNode->pointers[0];
                rightNode->numKeys--;
                rightNode->pointers[rightNode->numKeys] = rightNode->pointers[rightNode->numKeys + 1];
                rightNode->pointers[rightNode->numKeys + 1] = nullptr;
                for (int i = 0; i < rightNode->numKeys; i++) {
                    rightNode->keys[i] = rightNode->keys[i + 1];
                    rightNode->pointers[i] = rightNode->pointers[i + 1];
                }
                parent->keys[rightSibling - 1] = rightNode->keys[0];
                return;
            }
        }
        //try merge with left
        if (leftSibling >= 0) {
            Node *leftNode = (Node *) parent->pointers[leftSibling];
            for (int i = leftNode->numKeys, j = 0; j < cursor->numKeys; i++, j++) {
                leftNode->keys[i] = cursor->keys[j];
                leftNode->pointers[i] = cursor->pointers[j];
            }
            leftNode->numKeys += cursor->numKeys;
            leftNode->pointers[maxKeys] = cursor->pointers[maxKeys];
            removeInternalNode(parent->keys[leftSibling], parent, cursor);
            delete[] cursor->keys;
            delete[] cursor->pointers;
            delete cursor;
        } else if (rightSibling <= parent->numKeys) {
            Node *rightNode = (Node *) parent->pointers[rightSibling];
            for (int i = cursor->numKeys, j = 0; j < rightNode->numKeys; i++, j++) {
                cursor->keys[i] = rightNode->keys[j];
                cursor->pointers[i] = rightNode->pointers[j];
            }
            cursor->numKeys += rightNode->numKeys;
            cursor->pointers[maxKeys] = rightNode->pointers[maxKeys];
            cout << "Merging two leaf nodes\n";

            // Might need to removce internal node
            removeInternalNode(parent->keys[rightSibling - 1], parent, rightNode);
            delete[] rightNode->keys;
            delete[] rightNode->pointers;
            delete rightNode;
        }
    }
}


// Takes in the parent node, the child node to delete, and removes the child.
void BPlusTree::removeInternalNode(int key, Node *cursor, Node *child) {
    // If current parent is root
    if (cursor == root) {
        // If all keys in root is removed, change the root to its child
        if (cursor->numKeys == 1) {
            // Make the child to be new root
            if (cursor->pointers[1] == child) {
                // Delete the child completely
                delete[] child->keys;
                delete[] child->pointers;
                delete child;

                // New root will be parent's left pointer
                root = (Node *) cursor->pointers[0];

                delete[] cursor->keys;
                delete[] cursor->pointers;
                delete cursor;

                std::cout << "Root node changed." << endl;
                return;
            }
            // Else if left pointer in root (parent) contains the child, delete from there.
            else if (cursor->pointers[0] == child) {
                // Delete the child completely
                delete[] child->keys;
                delete[] child->pointers;
                delete child;


                // Set new root to be the parent's right pointer
                root = (Node *) cursor->pointers[1];

                delete[] cursor->keys;
                delete[] cursor->pointers;
                delete cursor;

                std::cout << "Root node changed." << endl;
                return;
            }
        }
    }

    // Parent is not root, need to delete internal node
    // Searching for key to delete in parent
    int pos;
    for (pos = 0; pos < cursor->numKeys; pos++) {
        if (cursor->keys[pos] == key) {
            break;
        }
    }

    // Shifting all keys forward to overwrite it
    for (int i = pos; i < cursor->numKeys; i++) {
        cursor->keys[i] = cursor->keys[i + 1];
    }

    // Search for pointer to be removed in parent node
    for (pos = 0; pos < cursor->numKeys + 1; pos++) {
        if (cursor->pointers[pos] == child) {
            break;
        }
    }

    // Shifting all pointers forward to overwrite it
    for (int i = pos; i < cursor->numKeys + 1; i++) {
        cursor->pointers[i] = cursor->pointers[i + 1];
    }

    // Update number of keys
    cursor->numKeys--;

    // Check if there's enough number of keys in parent
    if (cursor->numKeys >= (maxKeys + 1) / 2 - 1) {
        return;
    }

    // If we are the root, underflow doesn't matter.
    if (cursor == root) {
        return;
    }

    // If it is not root, we have to find the parent of this parent to get the left and right node.
    Node *parent = findParent((Node *) root, cursor);
    int leftSibling, rightSibling;

    for (pos = 0; pos < parent->numKeys + 1; pos++) {
        if (parent->pointers[pos] == cursor) {
            leftSibling = pos - 1;
            rightSibling = pos + 1;
            break;
        }
    }

    // Try to borrow a key from either the left or right node.
    if (leftSibling >= 0) {
        Node *leftNode = (Node *) parent->pointers[leftSibling];

        // Check if can borrow from left node
        if (leftNode->numKeys >= (maxKeys + 1) / 2) {
            // Move remaining keys and pointers backward.
            for (int i = cursor->numKeys; i > 0; i--) {
                cursor->keys[i] = cursor->keys[i - 1];
            }

            cursor->keys[0] = parent->keys[leftSibling];
            parent->keys[leftSibling] = leftNode->keys[leftNode->numKeys - 1];

            for (int i = cursor->numKeys + 1; i > 0; i--) {
                cursor->pointers[i] = cursor->pointers[i - 1];
            }

            cursor->pointers[0] = leftNode->pointers[leftNode->numKeys];

            // Update key numbers in keft node and current node
            cursor->numKeys++;
            leftNode->numKeys--;

            return;
        }
    }

    if (rightSibling <= parent->numKeys) {
        Node *rightNode = (Node *) parent->pointers[rightSibling];

        // Check if can borrow from right node.
        if (rightNode->numKeys >= (maxKeys + 1) / 2) {
            // Move remaining keys and pointers backward.
            cursor->keys[cursor->numKeys] = parent->keys[pos];
            parent->keys[pos] = rightNode->keys[0];

            for (int i = 0; i < rightNode->numKeys - 1; i++) {
                rightNode->keys[i] = rightNode->keys[i + 1];
            }

            cursor->pointers[cursor->numKeys + 1] = rightNode->pointers[0];

            for (int i = 0; i < rightNode->numKeys; ++i) {
                rightNode->pointers[i] = rightNode->pointers[i + 1];
            }

            // Update number of keys in cursor and right node
            cursor->numKeys++;
            rightNode->numKeys--;

            return;
        }
    }

    // Not possible to borrow, try merge with left node.
    if (leftSibling >= 0) {
        Node *leftNode = (Node *) parent->pointers[leftSibling];

        // Update left node's upper bound.
        leftNode->keys[leftNode->numKeys] = parent->keys[leftSibling];

        // Move current node's keys and pointers to left node.
        for (int i = leftNode->numKeys + 1, j = 0; j < cursor->numKeys; j++) {
            leftNode->keys[i] = cursor->keys[j];
        }

        for (int i = leftNode->numKeys + 1, j = 0; j < cursor->numKeys + 1; j++) {
            leftNode->pointers[i] = cursor->pointers[j];
            cursor->pointers[j] = nullptr;
        }

        // Update left node's last pointer.
        leftNode->numKeys += cursor->numKeys + 1;
        cursor->numKeys = 0;

        // Might need to update internal node.
        removeInternalNode(parent->keys[leftSibling], parent, cursor);
    }
    // Not possible to borrow, try merge with right node.
    else if (rightSibling <= parent->numKeys) {
        Node *rightNode = (Node *) parent->pointers[rightSibling];

        // Update current node's upper bound.
        cursor->keys[cursor->numKeys] = parent->keys[rightSibling - 1];

        // Move right node's keys and pointers to current node.
        for (int i = cursor->numKeys + 1, j = 0; j < rightNode->numKeys; j++) {
            cursor->keys[i] = rightNode->keys[j];
        }

        for (int i = cursor->numKeys + 1, j = 0; j < rightNode->numKeys + 1; j++) {
            cursor->pointers[i] = rightNode->pointers[j];
            rightNode->pointers[j] = nullptr;
        }

        cursor->numKeys += rightNode->numKeys + 1;
        rightNode->numKeys = 0;

        // Might need to update internal node.
        removeInternalNode(parent->keys[rightSibling - 1], parent, rightNode);
    }
}

int BPlusTree::checkDuplicate(int key) {
    vector<void *> vector_arr = searchRange(key, key);
    return vector_arr.size();
}
