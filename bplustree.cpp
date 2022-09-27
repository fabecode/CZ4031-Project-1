#include "bplustree.h"
#include "memory.h"

#include <iostream>
#include <tuple>
#include <unordered_map>
#include <cstring>
#include <array>
#include <vector>
#include <cmath>
#include <algorithm>

int MAX = 3;

using namespace std;

// bool myNullPtr = false;

Node::Node(int maxKeys) {
    // Initialize array with key and ptr
    keys = new float[maxKeys];
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
    pointers = new void *[maxKeys + 1];

    for (int i=0; i<maxKeys+1; i++) {
        pointers[i] = nullptr;
    }
    isLeaf = false;
    numKeys = 0;
}

BPlusTree::BPlusTree(int blocksize) {
    // size of block - isLeaf and numKeys
    blocksize = (int) (blocksize - sizeof(bool)- sizeof(int));
    // change maxkeys!!!!!!!!!!!!
    maxKeys = 5;
    // maxKeys = 23;
    if (maxKeys == 0) {
        throw std::overflow_error("Overflow Error: Too much key & ptr in 1 node");
    }
    // Initialize node
    root = nullptr;
    numNodes = 0;
}

// Insert a record into the B+ Tree index. Key: Record's avgRating, Value: {blockAddress, offset}.
void BPlusTree::insert(void *address, float key) {
    if (BPlusTree::root == nullptr) {
        OverflowNode *newNode = new OverflowNode(maxKeys);
        newNode->pointers[newNode->numKeys] = address;
        newNode->isLeaf = false;
        newNode->numKeys += 1;
        BPlusTree::root = new Node(BPlusTree::maxKeys);
        BPlusTree::root->isLeaf = true;
        BPlusTree::root->pointers[0] = newNode;
        BPlusTree::root->pointers[maxKeys+1] = nullptr;
        BPlusTree::root->keys[0] = key;
        BPlusTree::root->numKeys += 1;
        // everytime we create new node increment our counter
        BPlusTree::numNodes += 1;
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
        //check if its a duplicate key
        if (cursor->keys[i] == key) {
            OverflowNode *overflow = (OverflowNode *) cursor->pointers[i];
            // if this new node still has space
            if (overflow->numKeys + 1 < maxKeys) {
                overflow->pointers[overflow->numKeys] = address;
                overflow->numKeys += 1;
            } else { // create a new overflow node
                OverflowNode *newNode = new OverflowNode(maxKeys);
                newNode->isLeaf = false;
                newNode->pointers[newNode->numKeys] = address;
                newNode->numKeys += 1;
                overflow->pointers[overflow->numKeys] = newNode;
                // do we also count overflow node?
                // BPlusTree::numNodes += 1;
            }
        } else {
            // not a duplicate, we try to insert
            // do we also count overflow node?
            // BPlusTree::numNodes += 1;
            OverflowNode *newNode = new OverflowNode(maxKeys);
            newNode->pointers[newNode->numKeys] = address;
            newNode->numKeys += 1;
            // there is still space in this current node
            if (cursor->numKeys + 1 <= maxKeys) {
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
                cursor->pointers[i] = newNode;
                cursor->keys[i] = key;
                cursor->numKeys += 1;
            } else { // no space, we need to split the node
                // update numNodes
                BPlusTree::numNodes += 1;
                Node *tNode = new Node(maxKeys);
                tNode->isLeaf = true;
                tNode->pointers[maxKeys] = cursor->pointers[maxKeys];
                cursor->pointers[maxKeys] = tNode;
                // create a temp vector to hold the maxKeys + 1 items
                std::vector<pair<float, void*>> tpointers;
                // push the new item in
                pair<float, void*> newPair = make_pair(key, newNode);
                tpointers.push_back(newPair);
                // move all the items in the node in
                for (i=0; i<cursor->numKeys; i++) {
                    tpointers.push_back(make_pair(cursor->keys[i], cursor->pointers[i]));
                }
                // sort the temporary vector so that all the items are lined up correctly
                sort(tpointers.begin(), tpointers.end(), [&](pair<float, void*> p1, pair<float, void*> p2) {
                    return p1.first < p2.first;
                });
                // split the node in the middle, the left having ceil(maxKeys+1) / 2, the right having floor(maxKeys+1)/2
                int middle = ceil((maxKeys + 1)) / 2, j=0;
                for (i=0; i<tpointers.size(); i++) {
                    if (i < middle) {
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
                cursor->numKeys = middle;
                tNode->numKeys = (maxKeys + 1) / 2;
                // If we are at root (aka root == leaf), then we need to make a new parent root.
                int check = 0;
                float internalKey = tNode->keys[0];
                // not sure if we need this
                //while (search(tNode->keys[check], false, false) != NULL && check < tNode->numKeys) {
                //    check++;
                //}
                //internalKey = (check == tNode->numKeys) ? std::nan("") : tNode->keys[check];

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
void BPlusTree::insertInternal(float key, Node *cursor, Node *child) {

    if (cursor->numKeys  + 1 <= maxKeys) {
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
        float tempKeyList[maxKeys + 1];
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

std::vector<void *> BPlusTree::searchNumVotes(float lowerBoundKey, float upperBoundKey) {
    //search logic
    if (root == NULL) {
        throw std::logic_error("Tree is empty");
    } else {
        Node *cursor = root;
        //in the following while loop, cursor will travel to the leaf node possibly consisting the key
        while (cursor->isLeaf == false) {
            t.push_back(cursor);
            for (int i = 0; i < cursor->numKeys; i++) {
                if (isnan(cursor->keys[i]) || lowerBoundKey < cursor->keys[i]) {//Keep looping till data hit upper bound and stop
                    for (int j = 0; j < cursor->numKeys; j++) {
                        cout << cursor->keys[j] << " ";
                    }
                    cout << "\n";

                    cursor = (Node *) cursor->pointers[i];
                    //Add new feature to push data into vector for comparison against logic later

                    break;
                }
                if (i == cursor->numKeys - 1) {
                    for (int j = 0; j < cursor->numKeys; j++) {
                        cout << cursor->keys[j] << " ";
                    }
                    cout << "\n";

                    cursor = (Node *) cursor->pointers[i + 1];
                    break;
                }
            }
        }

        //in the following for loop, we search for the key if it exists
        // cursor will be in the correct leaf node - vector is a dynamic array that stores all the addresses

        std::vector<void *> records;
        cout << "vector initialized" << endl;
        int i = 0;
        int x = 0;
        float key = cursor->keys[0];
        for (int j = 0; j < (int) cursor->numKeys; j++) {
            cout << cursor->keys[j] << " ";
        }
        cout << endl;
        // move cursor to correct lower bound
        while (key < lowerBoundKey && i < cursor->numKeys) {
            key = cursor->keys[i];
            // cout << "key:" << key << " i:" << i << endl;
            i++;
        }
        i -= 1;
        cout << "Key here: " << cursor->keys[i] << endl;
        cout << "key:" << key << ",lowerBoundKey: " << lowerBoundKey << ", upperBoundKey:" << upperBoundKey << endl;

        /*start traversing through all the leaf till we meet upper bound*/
        for (x = i; x < cursor->numKeys; x++) {
            if (key <= upperBoundKey) {
                records.push_back(cursor->pointers[x]);
                key = cursor->keys[x];
            } else {
                break;
            }

            cursor = (Node *) cursor->pointers[cursor->numKeys];
            i = 0;
            key = cursor->keys[i];
            while (isnan(cursor->numKeys) && cursor->isLeaf == false) {
                cursor = (Node *) cursor->pointers[cursor->numKeys + 1];
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
        cout << "Enter leaf\n";
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

void BPlusTree::remove(float key) {

    if (root == NULL) {
        cout << "Tree empty\n";
    } else {
        Node *cursor = root;
        Node *parent;
        int leftSibling, rightSibling;

        //searching for the correct leaf node
        while (cursor->isLeaf == false) {
            for (int i = 0; i < cursor->numKeys; i++) {
                parent = cursor;
                leftSibling = i - 1;
                rightSibling = i + 1;
                if (key < cursor->keys[i]) {
                    cursor = (Node *) cursor->pointers[i];
                    break;
                }
                if (i == cursor->numKeys - 1) {
                    leftSibling = i;
                    rightSibling = i + 2;
                    cursor = (Node *) cursor->pointers[i + 1];
                    break;
                }
            }
        }
        //searching for the correct key in the particular leaf node
        bool found = false;
        int pos;
        for (pos = 0; pos < cursor->numKeys; pos++) {
            if (cursor->keys[pos] == key) {
                found = true;
                //check duplicate?
                break;
            }
        }
        if (!found) {
            cout << "Not found\n";
            return;
        }
        for (int i = pos; i < cursor->numKeys; i++) {
            cursor->keys[i] = cursor->keys[i + 1];
        }
        cursor->numKeys--;
        if (cursor == root) {
            for (int i = 0; i < MAX + 1; i++) {
                cursor->pointers[i] = NULL;
            }
            if (cursor->numKeys == 0) {
                cout << "Tree died\n";
                delete[] cursor->keys;
                delete[] cursor->pointers;
                delete cursor;
                root = NULL;
            }
            return;
        }
        cursor->pointers[cursor->numKeys] = cursor->pointers[cursor->numKeys + 1];
        cursor->pointers[cursor->numKeys + 1] = NULL;
        if (cursor->numKeys >= (MAX + 1) / 2) {
            return;
        }
        if (leftSibling >= 0) {
            Node *leftNode = (Node *) parent->pointers[leftSibling];
            if (leftNode->numKeys >= (MAX + 1) / 2 + 1) {
                for (int i = cursor->numKeys; i > 0; i--) {
                    cursor->keys[i] = cursor->keys[i - 1];
                }
                cursor->numKeys++;
                cursor->pointers[cursor->numKeys] = cursor->pointers[cursor->numKeys - 1];
                cursor->pointers[cursor->numKeys - 1] = NULL;
                cursor->keys[0] = leftNode->keys[leftNode->numKeys - 1];
                leftNode->numKeys--;
                leftNode->pointers[leftNode->numKeys] = cursor;
                leftNode->pointers[leftNode->numKeys + 1] = NULL;
                parent->keys[leftSibling] = cursor->keys[0];
                return;
            }
        }
        if (rightSibling <= parent->numKeys) {
            Node *rightNode = (Node *) parent->pointers[rightSibling];
            if (rightNode->numKeys >= (MAX + 1) / 2 + 1) {
                cursor->numKeys++;
                cursor->pointers[cursor->numKeys] = cursor->pointers[cursor->numKeys - 1];
                cursor->pointers[cursor->numKeys - 1] = NULL;
                cursor->keys[cursor->numKeys - 1] = rightNode->keys[0];
                rightNode->numKeys--;
                rightNode->pointers[rightNode->numKeys] = rightNode->pointers[rightNode->numKeys + 1];
                rightNode->pointers[rightNode->numKeys + 1] = NULL;
                for (int i = 0; i < rightNode->numKeys; i++) {
                    rightNode->keys[i] = rightNode->keys[i + 1];
                }
                parent->keys[rightSibling - 1] = rightNode->keys[0];
                return;
            }
        }
        if (leftSibling >= 0) {
            Node *leftNode = (Node *) parent->pointers[leftSibling];
            for (int i = leftNode->numKeys, j = 0; j < cursor->numKeys; i++, j++) {
                leftNode->keys[i] = cursor->keys[j];
            }
            leftNode->pointers[leftNode->numKeys] = NULL;
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
            }
            cursor->pointers[cursor->numKeys] = NULL;
            cursor->numKeys += rightNode->numKeys;
            cursor->pointers[cursor->numKeys] = rightNode->pointers[rightNode->numKeys];
            cout << "Merging two leaf nodes\n";
            removeInternal(parent->keys[rightSibling - 1], parent, rightNode);
            delete[] rightNode->keys;
            delete[] rightNode->pointers;
            delete rightNode;
        }
    }
    //Commented out to test linked list deletion
    /*
    // set numNodes before deletion
   //numNodes = index->getNumBlocks();
   // vector<void *> temp = searchNumVotes(key, key);
    // Tree is empty.
    if (root == nullptr)
    {
      throw std::logic_error("Tree is empty!");
    }
    else
    {

      Node *cursor = root;
      Node *parent;                          // Keep track of the parent as we go deeper into the tree in case we need to update it.
      int leftSibling, rightSibling; // Index of left and right child to borrow from.

      bool findkey = false;
      while (true){

        parent = cursor;

        for (int i=0; i<cursor->numKeys; i++){
          // Keep track of left and right to borrow.
          leftSibling = i - 1;
          rightSibling = i + 1;

          cout << "i:" << i << " cursor->pointers[i]: " << cursor->pointers[i] << endl;
          Node* childcursor = (Node*)cursor->pointers[i];

          if(childcursor->isLeaf){
            for(int j = 0; j < childcursor->numKeys; j++){
              if(key == childcursor->keys[j]){
                findkey == true;
                break;}
              }
          }
          cursor = (Node*)cursor->pointers[i];
        }
        cout << "error not found\n";
        break;

        // if (i == cursor->numKeys - 1)
        //   {
        //     leftSibling = i;
        //     rightSibling = i + 2;

        //     cursor = (Node*)cursor->pointers[i+1];
        //     break;
        //   }

        // for(int i = 0; i < cursor->numKeys; i++){
        // Node* cursor2 = (Node*)cursor->pointers[i];
        // while(cursor2->isLeaf != true){
        //   cursor2=cursor2->pointers[]
        //   for(int i = 0; i < cursor->numKeys;i++){
        //     if(key == cursor.keys[i])break;
        //     //else cursor = parent;
        //   }

        // }
        // }


      //}
      // While not leaf, keep following the nodes to correct key.
      while (cursor->isLeaf == false)
      {
        // Set the parent of the node (in case we need to assign new child later), and its disk address.
        parent = cursor;

        // Check through all keys of the node to find key and pointer to follow downwards.
        for (int i = 0; i < cursor->numKeys; i++)
        {
          // Keep track of left and right to borrow.
          leftSibling = i - 1;
          rightSibling = i + 1;

          // If key is lesser than current key, go to the left pointer's node.
          if (isnan(cursor->keys[i]) || key < cursor->keys[i])
          {
            cursor = (Node*)cursor->pointers[i];
            break;
          }
          // Else if key larger than all keys in the node, go to last pointer's node (rightmost).
          if (i == cursor->numKeys - 1)
          {
            leftSibling = i;
            rightSibling = i + 2;

            cursor = (Node*)cursor->pointers[i+1];
            break;
          }
        }
        //cout << "leftsibli" << leftSibling << " rightsib" << rightSibling << endl;

      }

      // now that we have found the leaf node that might contain the key, we will try and find the position of the key here (if exists)
      // search if the key to be deleted exists in this bplustree
      bool found = false;
      int pos=0;
      // also works for duplicates
      Node *temp = cursor;
      while(true){
        if(key == temp->keys[pos]){
          found = true;
          break;
        }
        if(key < temp->keys[pos]){
          break;
        }
        if(pos == temp->numKeys-1){
          temp = (Node*) temp->pointers[temp->numKeys];
          pos = 0;
        }else
          pos++;
      }

      // If key to be deleted does not exist in the tree, return error.
      if (!found)
      {
        std::cout << "Can't find specified key " << key << " to delete!" << endl;

        // update numNodes and numNodesDeleted after deletion
        // int numNodesDeleted = numNodes - index->getNumBlocks();;
        // numNodes = index->getNumBlocks();;
        // return numNodesDeleted;
        return;
      }

      // pos is the position where we found the key.
      cursor = temp;
      //Node *temp2 = temp;
      int temppos = pos;
      int keyshiftcount=0;
      int nodeshiftcount = 0;

      while(true){
        keyshiftcount++;
        temppos ++;
        if(temppos == temp->numKeys || temp->keys[temppos] != key){
          for (int i = temppos-1; i < temp->numKeys ; i++)
          {
            temp->keys[i] = temp->keys[i + keyshiftcount];
            temp->pointers[i] = temp->pointers[i + keyshiftcount];
          }
          temp->numKeys -= keyshiftcount;
          // Move the last pointer forward (if any).
          temp->pointers[temp->numKeys] = temp->pointers[temp->numKeys + keyshiftcount];

          for (int i = temp->numKeys + 1 ; i < maxKeys + 1; i++)
          {
            temp->pointers[i] = nullptr;
          }
          if(temp->keys[temppos] != key){
            nodeshiftcount++;
            break;
          }

          temp = (Node*)temp->pointers[temp->numKeys];
          temppos = 0;
          keyshiftcount = 0;
          //if(temp->keys[temppos]==key)
          nodeshiftcount++;

        }

      // }
      // cout << nodeshiftcount << "nodeshiftcnt" << endl;
      // cout << "leftsibli" << leftSibling << " rightsib" << rightSibling << endl;
      // If current node is root, check if tree still has keys.
      if (cursor == root)
      {
        if (cursor->numKeys == 0)
        {
          // Delete the entire root node and deallocate it.
          std::cout << "Congratulations! You deleted the entire index!" << endl;

          // Reset root pointers in the B+ Tree.
          root = nullptr;

        }
        std::cout << "Successfully deleted " << key << endl;
        numNodes--;

        // update numNodes and numNodesDeleted after deletion
        //int numNodesDeleted = numNodes - index->getAllocated();

        //return numNodesDeleted;
      }

      for(int n = 0; n < nodeshiftcount; n++){
        // If we didn't delete from root, we check if we have minimum keys ⌊(n+1)/2⌋ for leaf.
        if (cursor->numKeys >= (maxKeys + 1) / 2)
        {
          // No underflow, so we're done.
          //std::cout << "Successfully deleted " << key << "on node" << endl;

          // update numNodes and numNodesDeleted after deletion
          // int numNodesDeleted = numNodes - index->getAllocated();
          // return numNodesDeleted;
          cursor = (Node*)cursor->pointers[cursor->numKeys];
          cout << "not underflowwww\n" ;
          continue;
        }


        // If we reach here, means we have underflow (not enough keys for balanced tree).
        // Try to take from left sibling (node on same level) first.
        // Check if left sibling even exists.
        if (leftSibling >= 0)
        {
          // Load in left sibling from disk.
          Node *leftNode = (Node*)parent->pointers[leftSibling];

          // Check if we can steal (ahem, borrow) a key without underflow.
          if (leftNode->numKeys >= (maxKeys + 1) / 2 + 1)
          {
            // We will insert this borrowed key into the leftmost of current node (smaller).

            // Shift last pointer back by one first.
            cursor->pointers[cursor->numKeys + 1] = cursor->pointers[cursor->numKeys];

            // Shift all remaining keys and pointers back by one.
            for (int i = cursor->numKeys; i > 0; i--)
            {
              cursor->keys[i] = cursor->keys[i - 1];
              cursor->pointers[i] = cursor->pointers[i - 1];
            }

            // Transfer borrowed key and pointer (rightmost of left node) over to current node.
            cursor->keys[0] = leftNode->keys[leftNode->numKeys - 1];
            cursor->pointers[0] = leftNode->pointers[leftNode->numKeys - 1];
            cursor->numKeys++;
            leftNode->numKeys--;

            // Update left sibling (shift pointers left)
            leftNode->pointers[cursor->numKeys] = leftNode->pointers[cursor->numKeys + 1];

            // Update parent node's key
            //!!!check duplicate TT
            if(search(cursor->keys[0], false, false) == cursor ){
              parent->keys[leftSibling] = cursor->keys[0];
            }
            else{
              int check = 0;
              while( search(cursor->keys[check],false,false) != cursor && check < cursor->numKeys){
                check++;
              }
              parent->keys[leftSibling] =  (check == cursor->numKeys)? std::nan(""):cursor->keys[check];

            }

            // update numNodes and numNodesDeleted after deletion
            // int numNodesDeleted = numNodes - index->getAllocated();
            // numNodes = index->getAllocated();
            // return numNodesDeleted;
            return;
          }
        }

        // If we can't take from the left sibling, take from the right.
        // Check if we even have a right sibling.
        if (rightSibling <= parent->numKeys)
        {
          // If we do, load in right sibling from disk.
          Node *rightNode = (Node*)parent->pointers[rightSibling];

          // Check if we can steal (ahem, borrow) a key without underflow.
          if (rightNode->numKeys >= (maxKeys + 1) / 2 + 1)
          {

            // We will insert this borrowed key into the rightmost of current node (larger).
            // Shift last pointer back by one first.
            cursor->pointers[cursor->numKeys + 1] = cursor->pointers[cursor->numKeys];

            // No need to shift remaining pointers and keys since we are inserting on the rightmost.
            // Transfer borrowed key and pointer (leftmost of right node) over to rightmost of current node.
            cursor->keys[cursor->numKeys] = rightNode->keys[0];
            cursor->pointers[cursor->numKeys] = rightNode->pointers[0];
            cursor->numKeys++;
            rightNode->numKeys--;

            cout<<rightNode->keys[0]<<"\n";

            // Update right sibling (shift keys and pointers left)
            for (int i = 0; i < rightNode->numKeys; i++)
            {
              rightNode->keys[i] = rightNode->keys[i + 1];
              rightNode->pointers[i] = rightNode->pointers[i + 1];
            }

            // Move right sibling's last pointer left by one too.
            rightNode->pointers[cursor->numKeys] = rightNode->pointers[cursor->numKeys + 1];

            // Update parent node's key to be new lower bound of right sibling.
            if(search(cursor->keys[0], false, false) == cursor ){\
              parent->keys[rightSibling - 1] = rightNode->keys[0];
            }
            else{
              int check = 0;
              while( search(cursor->keys[check],false,false) != cursor && check < cursor->numKeys){
                check++;
              }
              parent->keys[rightSibling - 1] = (check == cursor->numKeys)? std::nan(""):rightNode->keys[0];

            }
            // update numNodes and numNodesDeleted after deletion
            // int numNodesDeleted = numNodes - index->getAllocated();
            // numNodes = index->getAllocated();
            // return numNodesDeleted;
            return;
          }
        }

        // If we reach here, means no sibling we can steal from.
        // To resolve underflow, we must merge nodes.

        // If left sibling exists, merge with it.
        if (leftSibling >= 0)
        {
          // Load in left sibling from disk.
          Node *leftNode = (Node*)parent->pointers[leftSibling];

          // Transfer all keys and pointers from current node to left node.
          // Note: Merging will always suceed due to ⌊(n)/2⌋ (left) + ⌊(n-1)/2⌋ (current).
          for (int i = leftNode->numKeys, j = 0; j < cursor->numKeys; i++, j++)
          {
            leftNode->keys[i] = cursor->keys[j];
            leftNode->pointers[i] = cursor->pointers[j];
          }

          // Update variables, make left node last pointer point to the next leaf node pointed to by current.
          leftNode->numKeys += cursor->numKeys;
          leftNode->pointers[leftNode->numKeys] = cursor->pointers[cursor->numKeys];

          // We need to update the parent in order to fully remove the current node.
          removeInternal(parent->keys[leftSibling], (Node *)parent, (Node *)cursor);

          // Now that we have updated parent, we can just delete the current node from disk.
          delete[] cursor->keys;
          delete[] cursor->pointers;
          //delete cursor;
        }
        // If left sibling doesn't exist, try to merge with right sibling.
        else if (rightSibling <= parent->numKeys)
        {
          // Load in right sibling from disk.
          Node *rightNode = (Node*)parent->pointers[rightSibling];

          // Note we are moving right node's stuff into ours.
          // Transfer all keys and pointers from right node into current.
          // Note: Merging will always suceed due to ⌊(n)/2⌋ (left) + ⌊(n-1)/2⌋ (current).
          for (int i = cursor->numKeys, j = 0; j < rightNode->numKeys; i++, j++)
          {
            cursor->keys[i] = rightNode->keys[j];
            cursor->pointers[i] = rightNode->pointers[j];
          }

          // Update variables, make current node last pointer point to the next leaf node pointed to by right node.
          cursor->numKeys += rightNode->numKeys;
          cursor->pointers[cursor->numKeys] = rightNode->pointers[rightNode->numKeys];

          // if(search(cursor->keys[0], false, false) == cursor ){
          //     parent->keys[leftSibling] = cursor->keys[0];
          //   }
          //   else{
          //     int check = 0;
          //     while( search(cursor->keys[check],false,false) != cursor && check < cursor->numKeys){
          //       check++;
          //     }
          //     parent->keys[leftSibling] =  (check == cursor->numKeys)? std::nan(""):cursor->keys[check];

          //   }

          // We need to update the parent in order to fully remove the right node.
          removeInternal(parent->keys[rightSibling - 1], parent, rightNode);

          // Now that we have updated parent, we can just delete the right node from disk.
          delete[] rightNode->keys;
          delete[] rightNode->pointers;
          //delete cursor;
        }
        cursor = (Node*)cursor->pointers[cursor->numKeys];
      }

      // update numNodes and numNodesDeleted after deletion
      // int numNodesDeleted = numNodes - index->getAllocated();
      // numNodes = index->getAllocated();
      // return numNodesDeleted;
      return;
      }
    }
  */
}


// Takes in the parent disk address, the child address to delete, and removes the child.
void BPlusTree::removeInternal(float key, Node *cursor, Node *child) {
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
                // Load left pointer into main memory and update root.
                root = (Node *) cursor->pointers[0];

                delete[] cursor->keys;
                delete[] cursor->pointers;
                delete cursor;

                // Nothing to save to disk. All updates happened in main memory.
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
                // Load right pointer into main memory and update root.
                root = (Node *) cursor->pointers[1];

                delete[] cursor->keys;
                delete[] cursor->pointers;
                delete cursor;

                // Nothing to save to disk. All updates happened in main memory.
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
        if (cursor->keys[pos] == key || (isnan(cursor->keys[pos]) && isnan(key))) {
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
            leftNode->pointers[cursor->numKeys] = leftNode->pointers[cursor->numKeys + 1];

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

int BPlusTree::checkDuplicate(float key) {
    vector<void *> vector_arr = searchNumVotes(key, key);
    return vector_arr.size();

}
// int main() {
//   BPlusTree node;
//   char a = 't';

//  node.insert(&a,12);
//  node.insert(&a,11);
//  node.insert(&a,10);
//  node.insert(&a,10);
//  node.insert(&a,9);
//  node.insert(&a,8);
// node.insert(&a,7);
//  node.insert(&a,6);
// //  node.remove(10);

//   node.display();
// }
