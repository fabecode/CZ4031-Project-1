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
    while (sizeof(float) + sizeof(void *) <= left) {
        maxKeys += 1;
        left -= (sizeof(float) + sizeof(void *));
    }
    // maxKeys = 5;
    // Initialize node
    root = nullptr;
    numNodes = 0;
}

// Insert a record into the B+ Tree index. Key: Record's avgRating, Value: {blockAddress, offset}.
void BPlusTree::insert(void *address, int key) {
    if (root == nullptr) {
        Node *newNode = new Node(maxKeys);
        newNode->keys[0] = key;
        newNode->isLeaf = true;
        OverflowNode *newOverflow = new OverflowNode(overflowSize);
        newOverflow->pointers[newOverflow->numKeys] = address;
        newOverflow->isLeaf = false;
        newNode->pointers[0] = newOverflow;
        root = newNode;
    } else {
        Node *cursor = BPlusTree::root;
        Node *parent;
        int i;
        // finding the correct leaf node to insert item
        while (!cursor->isLeaf) {
            parent = cursor;
            for (i=0; i<cursor->numKeys; i++) {
                if (cursor->keys[i] > key) {
                    cursor = (Node *) parent->pointers[i];
                    break;
                }

                if (i == cursor->numKeys - 1) {
                    cursor = (Node *) parent->pointers[i+1];
                    break;
                }
            }
        }

        // checks if key is already in leaf node
        // check if there is duplicates in the current leaf
        i = 0;
        // While we haven't reached the last key and the key we want to insert is larger than current key, keep moving forward.
        while (key > cursor->keys[i] && i < cursor->numKeys)i++;
        if (cursor->keys[i] == key) {
            OverflowNode *overflow = (OverflowNode *) cursor->pointers[i];
            //while (overflow->pointers[overflowSize - 1] != nullptr) {
            //    overflow = (OverflowNode *) overflow->pointers[overflowSize - 1];
            //}
            if (overflow->numKeys < overflowSize) {
                overflow->pointers[overflow->numKeys] = address;
                overflow->numKeys += 1;
            } else {
                OverflowNode *newOverflow = new OverflowNode(overflowSize);
                // overflow->pointers[overflowSize - 1] = newOverflow;
                newOverflow->pointers[newOverflow->numKeys] = address;
                newOverflow->numKeys += 1;
                newOverflow->pointers[overflowSize - 1] = cursor->pointers[i];
                cursor->pointers[i]  = newOverflow;
            }
        } else {
            OverflowNode *newOverflow = new OverflowNode(overflowSize);
            newOverflow->isLeaf = false;
            newOverflow->pointers[0] = address;
            if (cursor->numKeys < maxKeys) {
                for (i=0; i<cursor->numKeys; i++) {
                    if (cursor->keys[i] > key) {
                        break;
                    }
                }
                // move items back
                for (int j=cursor->numKeys; j>i; j--) {
                    cursor->pointers[j] = cursor->pointers[j-1];
                    cursor->keys[j] = cursor->keys[j-1];
                }
                // point it to the overflow node
                cursor->pointers[i] = newOverflow;
                cursor->keys[i] = key;
                cursor->numKeys += 1;
            } else {
                // update numNodes
                BPlusTree::numNodes += 1;
                Node *tNode = new Node(maxKeys);
                tNode->isLeaf = true;
                //tNode->pointers[maxKeys+1] = cursor->pointers[maxKeys+1];
                //cursor->pointers[maxKeys+1] = tNode;
                tNode->pointers[maxKeys] = cursor->pointers[maxKeys];
                cursor->pointers[maxKeys] = tNode;
                // create a temp vector to hold the maxKeys + 1 items
                std::vector<pair<int, void*>> tpointers;
                // push the new item in
                tpointers.push_back(make_pair(key, newOverflow));
                // move all the items in the node in
                for (i=0; i<cursor->numKeys; i++) {
                    tpointers.push_back(make_pair(cursor->keys[i], cursor->pointers[i]));
                }
                // sort the temporary vector so that all the items are lined up correctly
                sort(tpointers.begin(), tpointers.end(), [&](pair<int, void*> p1, pair<int, void*> p2) {
                    return p1.first < p2.first;
                });
                // split the node in the middle, the left having ceil(maxKeys+1) / 2, the right having floor(maxKeys+1)/2
                cursor->numKeys = ceil((maxKeys + 1) / 2);
                tNode->numKeys = (maxKeys + 1) - ceil((maxKeys + 1) / 2);
                int j=0;
                for (i=0; i<tpointers.size(); i++) {
                    if (i < cursor->numKeys) {
                        cursor->pointers[i] = tpointers[i].second;
                        cursor->keys[i] = tpointers[i].first;
                    } else {
                        tNode->pointers[j] = tpointers[i].second;
                        tNode->keys[j] = tpointers[i].first;
                        j += 1;
                        cursor->pointers[i] = nullptr;
                        cursor->keys[i] = 0;
                    }
                }
                // set the correct numKeys value for the new left and right node
                //cursor->numKeys = middle;
                //tNode->numKeys = (maxKeys + 1) / 2;
                // If we are at root (aka root == leaf), then we need to make a new parent root.
                int internalKey = tNode->keys[0];

                if (cursor == root) {
                    // created new root, update numNodes
                    BPlusTree::numNodes += 1;
                    Node *newRoot = new Node(maxKeys);
                    // We need to set the new root's key to be the left bound of the right child.
                    newRoot->keys[0] = internalKey;

                    // Point the new root's children as the existing node and the new node.
                    newRoot->pointers[0] = cursor;
                    newRoot->pointers[1] = tNode;

                    // Update new root's variables.
                    newRoot->isLeaf = false;
                    newRoot->numKeys = 1;

                    // Update the root node
                    root = newRoot;
                } else { // If we are not at the root, we need to insert a new parent in the middle levels of the tree.
                    insertInternal(internalKey, parent, tNode);
                }
            }
        }
    }
}

// Updates the parent node to point at both child nodes, and adds a parent node if needed.
// Takes the lower bound of the right child, and the main memory address of the parent and the new child,
// as well as disk address of parent and new child.
void BPlusTree::insertInternal(int key, Node *cursor, Node *child) {

    if (cursor->numKeys < maxKeys) {
        // t
        // Iterate through the parent to see where to put in the lower bound key for the new child.
        int i = 0;
        if (isnan(key)) {
            while (child->keys[child->numKeys] > cursor->keys[i])i++;
        } else {
            while (key > cursor->keys[i] && i < cursor->numKeys)i++;
        }


        // Now we have i, the index to insert the key in. Bubble swap all keys back to insert the new child's key.
        // We use numKeys as index since we are going to be inserting a new key.
        for (int j = cursor->numKeys; j > i; j--) {
            cursor->keys[j] = cursor->keys[j - 1];
        }

        // Shift all pointers one step right (right pointer of key points to lower bound of key).
        for (int j = cursor->numKeys + 1; j > i + 1; j--) {
            cursor->pointers[j] = cursor->pointers[j - 1];
        }

        // Add in new child's lower bound key and pointer to the parent.
        cursor->keys[i] = key;
        cursor->numKeys++;
        cursor->pointers[i + 1] = child;
    } else {
        // If parent node doesn't have space, we need to recursively split parent node and insert more parent nodes.

        // Make new internal node (split this parent node into two).
        // Note: We DO NOT add a new key, just a new pointer!
        Node *newInternal = new Node(maxKeys);
        numNodes++;

        // Same logic as above, keep a temp list of keys and pointers to insert into the split nodes.
        // Now, we have one extra pointer to keep track of (new child's pointer).
        int tempKeyList[maxKeys + 1];
        Node *tempPointerList[maxKeys + 2];

        // Copy all keys into a temp key list.
        // Note all keys are filled so we just copy till maxKeys.
        for (int i = 0; i < maxKeys; i++) {
            tempKeyList[i] = cursor->keys[i];
        }

        // Copy all pointers into a temp pointer list.
        // There is one more pointer than keys in the node so maxKeys + 1.
        for (int i = 0; i < maxKeys + 1; i++) {
            tempPointerList[i] = (Node *) cursor->pointers[i];
        }

        // Find index to insert key in temp key list.
        int i = 0;
        if (isnan(key)) {
            while (child->keys[child->numKeys] > tempKeyList[i])i++;
        } else {
            while (key > tempKeyList[i] && i < maxKeys)i++;
        }

        // Swap all elements higher than index backwards to fit new key.
        int j;
        for (int j = maxKeys; j > i; j--) {
            tempKeyList[j] = tempKeyList[j - 1];
        }

        // Insert new key into array in the correct spot (sorted).
        tempKeyList[i] = key;

        // Move all pointers back to fit new child's pointer as well.
        //2????
        for (int j = maxKeys + 1; j > i + 1; j--) {
            tempPointerList[j] = tempPointerList[j - 1];
        }

        // Insert a pointer to the child to the right of its key.
        tempPointerList[i + 1] = child;
        newInternal->isLeaf = false; // Can't be leaf as it's a parent.

        // Split the two new nodes into two. ⌊(n)/2⌋ keys for left.
        // For right, we drop the rightmost key since we only need to represent the pointer.
        cursor->numKeys = (maxKeys + 1) / 2;
        newInternal->numKeys = maxKeys - (maxKeys + 1) / 2;

        // Reassign keys into cursor from tempkeyslist to account for new child node
        for (int i = 0; i < cursor->numKeys; i++) {
            cursor->keys[i] = tempKeyList[i];
        }

        // Insert new keys into the new internal parent node.
        for (i = 0, j = cursor->numKeys + 1; i < newInternal->numKeys; i++, j++) {
            newInternal->keys[i] = tempKeyList[j];
        }

        for (i = 0; i < cursor->numKeys + 1; i++) {
            cursor->pointers[i] = tempPointerList[i];
        }

        // Insert pointers into the new internal parent node.
        for (i = 0, j = cursor->numKeys + 1; i < newInternal->numKeys + 1; i++, j++) {
            newInternal->pointers[i] = tempPointerList[j];
        }
        for (int i = cursor->numKeys; i < maxKeys; i++) {
            cursor->keys[i] = float();
        }

        for (int i = cursor->numKeys + 2; i < maxKeys + 1; i++) {
            cursor->pointers[i] = nullptr;
        }



        // assign the new child to the original parent
        //cursor->pointers[cursor->numKeys] = child;


        // If current cursor is the root of the tree, we need to create a new root.
        if (cursor == root) {
            Node *newRoot = new Node(maxKeys);
            // Update newRoot to hold the children.
            // Take the rightmost key of the old parent to be the root.
            // Although we threw it away, we are still using it to denote the leftbound of the old child.
            //newRoot->keys[0] = cursor->keys[cursor->numKeys];
            newRoot->keys[0] = tempKeyList[cursor->numKeys];

            // Update newRoot's children to be the previous two nodes
            newRoot->pointers[0] = cursor;
            newRoot->pointers[1] = newInternal;

            // Update variables for newRoot
            newRoot->isLeaf = false;
            newRoot->numKeys = 1;

            root = newRoot;

        }
            // Otherwise, parent is internal, so we need to split and make a new parent internally again.
            // This is done recursively if needed.
        else {
            insertInternal(tempKeyList[cursor->numKeys], findParent(root, cursor), newInternal);
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

std::vector<void *> BPlusTree::searchNumVotes(int lowerBoundKey, int upperBoundKey) {
    //search logic
    if (root == nullptr) {
        // return empty vector
        return {};
        // throw std::logic_error("Tree is empty");
    }
    Node *cursor = root;
    //in the following while loop, cursor will travel to the leaf node possibly consisting the key
    while (!cursor->isLeaf) {
        t.push_back(cursor);
        for (int i = 0; i < cursor->numKeys; i++) {
            if (lowerBoundKey <= cursor->keys[i]) {//Keep looping till data hit upper bound and stop
                cursor = (Node *) cursor->pointers[i];
                //Add new feature to push data into vector for comparison against logic later
                break;
            }
            if (i == cursor->numKeys - 1) {
                cursor = (Node *) cursor->pointers[i + 1];
                break;
            }
        }
    }

    //in the following for loop, we search for the key if it exists
    // cursor will be in the correct leaf node - vector is a dynamic array that stores all the addresses
    std::vector<void *> records;
    int i = 0;
    while (lowerBoundKey > cursor->keys[i] && i < cursor->numKeys)i++;
    for (; i<=cursor->numKeys; i++) {
        if (i == cursor->numKeys) {
            cursor = (Node *) cursor->pointers[maxKeys+1];
            i = 0;
        }
        if (cursor == nullptr) {
            break;
        }
        if (records.size() == 44) {
            cout << "here" << endl;
        }
        if (cursor->keys[i] > upperBoundKey) {
            break;
        }
        records.push_back(cursor->pointers[i]);
    }
    return records;
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

/*
int BPlusTree::getHeightData(Node* node){
    if(node->isLeaf==false){
        //return BPlusTree::getHeight((Node *) node->pointers[0])+1;
        return 1;
    }
    else if(node->isLeaf==true){
        return 1;
    }
    else{
        return NULL;
    }
}
*/

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

        //searching for the correct leaf node
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
        //searching for the correct key in the particular leaf node
        bool found = false;
        int pos;
        for (pos = 0; pos < cursor->numKeys; pos++) {
            if (cursor->keys[pos] == key) {
                found = true;
                break;
            }
        }
        if (!found) {
            cout << "Not found\n";
            return;
        }

        //Found the key, delete overflow nodes
        OverflowNode *curr = (OverflowNode*) cursor->pointers[pos];
        delete curr;

        //start shifting keys forward to replace it
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
            //What is this for????
            // for (int i = 0; i < MAX + 1; i++) {
            //     cursor->pointers[i] = nullptr;
            // }
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

                // //set current node's last pointers
                // cursor->pointers[cursor->numKeys] = cursor->pointers[cursor->numKeys - 1];
                // cursor->pointers[cursor->numKeys - 1] = NULL;
                
                leftNode->numKeys--;
                leftNode->pointers[leftNode->numKeys] = cursor;
                leftNode->pointers[leftNode->numKeys + 1] = nullptr;
                parent->keys[leftSibling] = cursor->keys[0];
                return;
            }
        }
        if (rightSibling <= parent->numKeys) {
            Node *rightNode = (Node *) parent->pointers[rightSibling];
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
            leftNode->pointers[leftNode->numKeys] = cursor->pointers[cursor->numKeys];
            removeInternal(parent->keys[leftSibling], parent, cursor);
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
            cursor->pointers[cursor->numKeys] = rightNode->pointers[rightNode->numKeys];
            cout << "Merging two leaf nodes\n";
            removeInternal(parent->keys[rightSibling - 1], parent, rightNode);
            delete[] rightNode->keys;
            delete[] rightNode->pointers;
            delete rightNode;
        }
    }
   
}


// Takes in the parent node, the child node to delete, and removes the child.
void BPlusTree::removeInternal(int key, Node *cursor, Node *child) {
    // If current parent is root
    if (cursor == root) {
        // If we have to remove all keys in root (as parent) we need to change the root to its child.
        if (cursor->numKeys == 1) {
            // If the larger pointer points to child, make it the new root.
            if (cursor->pointers[1] == child) {
                // Delete the child completely
                delete[] child->keys;
                delete[] child->pointers;
                delete child;

                // Set new root to be the parent's left pointer
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

    // If reach here, means parent is NOT the root.
    // Aka we need to delete an internal node (possibly recursively).
    int pos;


    // Search for key to delete in parent based on child's lower bound key.
    for (pos = 0; pos < cursor->numKeys; pos++) {
        if (cursor->keys[pos] == key) {
            break;
        }
    }

    // Delete the key by shifting all keys forward
    for (int i = pos; i < cursor->numKeys; i++) {
        cursor->keys[i] = cursor->keys[i + 1];
    }

    // Search for pointer to delete in parent
    // Remember pointers are on the RIGHT for non leaf nodes.
    for (pos = 0; pos < cursor->numKeys + 1; pos++) {
        if (cursor->pointers[pos] == child) {
            break;
        }
    }

    // Now move all pointers from that point on forward by one to delete it.
    for (int i = pos; i < cursor->numKeys + 1; i++) {
        cursor->pointers[i] = cursor->pointers[i + 1];
    }

    // Update numKeys
    cursor->numKeys--;

    // Check if there's underflow in parent
    // No underflow, life is good.
    if (cursor->numKeys >= (maxKeys + 1) / 2 - 1) {
        return;
    }

    // If we reach here, means there's underflow in parent's keys.
    // Try to steal some from neighbouring nodes.
    // If we are the root, underflow doesn't matter.
    if (cursor == root) {
        return;
    }

    // If not, we need to find the parent of this parent to get our siblings.
    // Pass in lower bound key of our child to search for it.
    Node *parent = findParent((Node *) root, cursor);
    int leftSibling, rightSibling;

    // Find left and right sibling of cursor, iterate through pointers.
    for (pos = 0; pos < parent->numKeys + 1; pos++) {
        if (parent->pointers[pos] == cursor) {
            leftSibling = pos - 1;
            rightSibling = pos + 1;
            break;
        }
    }

    // Try to borrow a key from either the left or right sibling.
    // Check if left sibling exists. If so, try to borrow.
    if (leftSibling >= 0) {
        // Load in left sibling from disk.

        Node *leftNode = (Node *) parent->pointers[leftSibling];

        // Check if we can steal (ahem, borrow) a key without underflow.
        // Non leaf nodes require a minimum of ⌊n/2⌋
        if (leftNode->numKeys >= (maxKeys + 1) / 2) {
            // We will insert this borrowed key into the leftmost of current node (smaller).
            // Shift all remaining keys and pointers back by one.
            for (int i = cursor->numKeys; i > 0; i--) {
                cursor->keys[i] = cursor->keys[i - 1];
            }

            // Transfer borrowed key and pointer to cursor from left node.
            // Basically duplicate cursor lower bound key to keep pointers correct.
            cursor->keys[0] = parent->keys[leftSibling];
            parent->keys[leftSibling] = leftNode->keys[leftNode->numKeys - 1];

            // Move all pointers back to fit new one
            for (int i = cursor->numKeys + 1; i > 0; i--) {
                cursor->pointers[i] = cursor->pointers[i - 1];
            }

            // Add pointers to cursor from left node.
            cursor->pointers[0] = leftNode->pointers[leftNode->numKeys];

            // Change key numbers
            cursor->numKeys++;
            leftNode->numKeys--;

            // Update left sibling (shift pointers left)
            //what is this forrrr???
            //leftNode->pointers[cursor->numKeys] = leftNode->pointers[cursor->numKeys + 1];

            return;
        }
    }

    // If we can't take from the left sibling, take from the right.
    // Check if we even have a right sibling.
    if (rightSibling <= parent->numKeys) {
        // If we do, load in right sibling from disk.
        Node *rightNode = (Node *) parent->pointers[rightSibling];
        // Check if we can steal (ahem, borrow) a key without underflow.
        if (rightNode->numKeys >= (maxKeys + 1) / 2) {
            // No need to shift remaining pointers and keys since we are inserting on the rightmost.
            // Transfer borrowed key and pointer (leftmost of right node) over to rightmost of current node.
            cursor->keys[cursor->numKeys] = parent->keys[pos];
            parent->keys[pos] = rightNode->keys[0];

            // Update right sibling (shift keys and pointers left)
            for (int i = 0; i < rightNode->numKeys - 1; i++) {
                rightNode->keys[i] = rightNode->keys[i + 1];
            }

            // Transfer first pointer from right node to cursor
            cursor->pointers[cursor->numKeys + 1] = rightNode->pointers[0];

            // Shift pointers left for right node as well to delete first pointer
            for (int i = 0; i < rightNode->numKeys; ++i) {
                rightNode->pointers[i] = rightNode->pointers[i + 1];
            }

            // Update numKeys
            cursor->numKeys++;
            rightNode->numKeys--;

            return;
        }
    }

    // If we reach here, means no sibling we can steal from.
    // To resolve underflow, we must merge nodes.

    // If left sibling exists, merge with it.
    if (leftSibling >= 0) {
        // Load in left sibling from disk.
        Node *leftNode = (Node *) parent->pointers[leftSibling];

        // Make left node's upper bound to be cursor's lower bound.
        leftNode->keys[leftNode->numKeys] = parent->keys[leftSibling];

        // Transfer all keys from current node to left node.
        // Note: Merging will always suceed due to ⌊(n)/2⌋ (left) + ⌊(n-1)/2⌋ (current).
        for (int i = leftNode->numKeys + 1, j = 0; j < cursor->numKeys; j++) {
            leftNode->keys[i] = cursor->keys[j];
        }

        // Transfer all pointers too.
        for (int i = leftNode->numKeys + 1, j = 0; j < cursor->numKeys + 1; j++) {
            leftNode->pointers[i] = cursor->pointers[j];
            cursor->pointers[j] = nullptr;
        }

        // Update variables, make left node last pointer point to the next leaf node pointed to by current.
        leftNode->numKeys += cursor->numKeys + 1;
        cursor->numKeys = 0;

        // Delete current node (cursor)
        // We need to update the parent in order to fully remove the current node.
        removeInternal(parent->keys[leftSibling], parent, cursor);
    }
        // If left sibling doesn't exist, try to merge with right sibling.
    else if (rightSibling <= parent->numKeys) {
        // Load in right sibling from disk.
        Node *rightNode = (Node *) parent->pointers[rightSibling];

        // Set upper bound of cursor to be lower bound of right sibling.
        cursor->keys[cursor->numKeys] = parent->keys[rightSibling - 1];

        // Note we are moving right node's stuff into ours.
        // Transfer all keys from right node into current.
        // Note: Merging will always suceed due to ⌊(n)/2⌋ (left) + ⌊(n-1)/2⌋ (current).
        for (int i = cursor->numKeys + 1, j = 0; j < rightNode->numKeys; j++) {
            cursor->keys[i] = rightNode->keys[j];
        }

        // Transfer all pointers from right node into current.
        for (int i = cursor->numKeys + 1, j = 0; j < rightNode->numKeys + 1; j++) {
            cursor->pointers[i] = rightNode->pointers[j];
            rightNode->pointers[j] = nullptr;
        }

        // Update variables
        cursor->numKeys += rightNode->numKeys + 1;
        rightNode->numKeys = 0;


        // Delete right node.
        // We need to update the parent in order to fully remove the right node.
        removeInternal(parent->keys[rightSibling - 1], parent, rightNode);
    }
}

int BPlusTree::checkDuplicate(int key) {
    vector<void *> vector_arr = searchNumVotes(key, key);
    return vector_arr.size();
}
