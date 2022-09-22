#include "bplustree.h"
#include "memory.h"

#include <iostream>
#include <tuple>
#include <unordered_map>
#include <cstring>
#include <array>
#include <vector>
#include <cmath>

using namespace std;

bool myNullPtr = false;

Node::Node(int maxKeys)
{
  // Initialize array with key and ptr
  keys = new float[maxKeys];
  pointers = new void*[maxKeys + 1];

  // for (int i = 0; i < maxKeys + 1; i++)
  // {
  //   Address nullAddress{(void *)myNullPtr, 0};
  //   pointers[i] = nullAddress;
  // }
  numKeys = 0;
}

BPlusTree::BPlusTree()
{

  maxKeys = 3;

  if (maxKeys == 0)
  {
    throw std::overflow_error("Overflow Error: Too much key & ptr in 1 node");
  }

  // Initialize node
  root = nullptr;


  degree = 0;
  numNodes = 0;

  // Initialize disk space 
  //this->disk = disk;
  //this->index = index;
}

// Insert a record into the B+ Tree index. Key: Record's avgRating, Value: {blockAddress, offset}.
void BPlusTree::insert(void *address, float key)
{
  // If no root exists, create a new B+ Tree root.
  if (root == nullptr)
  {
     // Create new node in main memory, set it to root, and add the key and values to it.
     
    root = new Node(maxKeys);
    root->keys[0] = key;
    root->isLeaf = true; // It is both the root and a leaf.
    root->numKeys = 1;
    root->pointers[0] = address; // Add record's disk address to pointer.

  }
  // Else if root exists already, traverse the nodes to find the proper place to insert the key.
  else
  {
    
    Node *cursor = root;
    Node *parent;                          // Keep track of the parent as we go deeper into the tree in case we need to update it.

    // While not leaf, keep following the nodes to correct key.
    while (cursor->isLeaf == false)
    {

      // Set the parent of the node (in case we need to assign new child later), and its disk address.
      parent = cursor;
      // Check through all keys of the node to find key and pointer to follow downwards.
      for (int i = 0; i < cursor->numKeys; i++)
      {
        // If key is lesser than current key, go to the left pointer's node.
        if (key < cursor->keys[i])
        {
          cursor = (Node*)cursor->pointers[i];
          break;
        }
        // Else if key larger than all keys in the node, go to last pointer's node (rightmost).
        if (i == cursor->numKeys - 1)
        {
          cursor = (Node*)cursor->pointers[i+1];
          break;
        }
      }
    }

    // When we reach here, it means we have hit a leaf node. Let's find a place to put our new record in.
    // If this leaf node still has space to insert a key, then find out where to put it.
    if (cursor->numKeys < maxKeys)
    {
      int i = 0;
      // While we haven't reached the last key and the key we want to insert is larger than current key, keep moving forward.
      while (key > cursor->keys[i] && i < cursor->numKeys)
      {
        i++;
      }

      // Update the last pointer to point to the previous last pointer's node. Aka maintain cursor -> Y linked list.
      void* next = cursor->pointers[cursor->numKeys];

      // Now i represents the index we want to put our key in. We need to shift all keys in the node back to fit it in.
      // Swap from number of keys + 1 (empty key) backwards, moving our last key back and so on. We also need to swap pointers.
      for (int j = cursor->numKeys; j > i; j--)
      {
        // Just do a simple bubble swap from the back to preserve index order.
        cursor->keys[j] = cursor->keys[j - 1];
        cursor->pointers[j] = cursor->pointers[j - 1];
      }

      // Insert our new key and pointer into this node.
      cursor->keys[i] = key;
      cursor->numKeys++;
      cursor->pointers[i] = address;
      //cursor->numKeys++;

      // Update leaf node pointer link to next node
      cursor->pointers[cursor->numKeys] = next;

    }
    // Overflow: If there's no space to insert new key, we have to split this node into two and update the parent if required.
    else
    {
      // Create a new leaf node to put half the keys and pointers in.
      Node *newLeaf = new Node(maxKeys);

      // Copy all current keys and pointers (including new key to insert) to a temporary list.
      float tempKeyList[maxKeys + 1];

      // We only need to store pointers corresponding to records (ignore those that points to other nodes).
      // Those that point to other nodes can be manipulated by themselves without this array later.
      void *tempPointerList[maxKeys + 1];
      Node *next = (Node*)cursor->pointers[cursor->numKeys];

      // Copy all keys and pointers to the temporary lists.
      int i = 0,j;
      for (i = 0; i < maxKeys; i++)
      {
        tempKeyList[i] = cursor->keys[i];
        tempPointerList[i] = cursor->pointers[i];
      }

      // Insert the new key into the temp key list, making sure that it remains sorted. Here, we find where to insert it.
      i = 0;
      while (key > tempKeyList[i] && i < maxKeys)
      {
        i++;
      }

      // Else no duplicate, insert new key.
      // The key should be inserted at index i in the temporary lists. Move all elements back.
      for (int j = maxKeys; j > i; j--)
      {
        // Bubble swap all elements (keys and pointers) backwards by one index.
        tempKeyList[j] = tempKeyList[j - 1];
        tempPointerList[j] = tempPointerList[j - 1];
      }

      // Insert the new key and pointer into the temporary lists.
      tempKeyList[i] = key;
      tempPointerList[i] = address;

      newLeaf->isLeaf = true;
      cursor->numKeys = (maxKeys + 1) / 2;
      newLeaf->numKeys = maxKeys + 1 - (maxKeys + 1) / 2;
      cursor->pointers[cursor->numKeys] = newLeaf;
      newLeaf->pointers[newLeaf->numKeys] = cursor->pointers[maxKeys];
      cursor->pointers[maxKeys] = NULL;


      // Now we need to deal with the rest of the keys and pointers.
      // Note that since we are at a leaf node, pointers point directly to records on disk.

      // Add in keys and pointers in both the existing node, and the new leaf node.
      // First, the existing node (cursor).
      for (i = 0; i < cursor->numKeys; i++)
      {
        cursor->keys[i] = tempKeyList[i];
        cursor->pointers[i] = tempPointerList[i];
      }

      // Then, the new leaf node. Note we keep track of the i index, since we are using the remaining keys and pointers.
      for (int j = 0; j < newLeaf->numKeys; i++, j++)
      {
        newLeaf->keys[j] = tempKeyList[i];
        newLeaf->pointers[j] = tempPointerList[i];
      }

      // If we are at root (aka root == leaf), then we need to make a new parent root.
      int check = 0;
      float internalKey;
      while( search(newLeaf->keys[check],false,false) != NULL && check < newLeaf->numKeys){
        check++;
      }
      internalKey =  (check == newLeaf->numKeys)? std::nan(""):newLeaf->keys[check];

      if (cursor == root)
      {
        Node *newRoot = new Node(maxKeys);

        // We need to set the new root's key to be the left bound of the right child.
        newRoot->keys[0] = internalKey;

        // Point the new root's children as the existing node and the new node.
        newRoot->pointers[0] = cursor;
        newRoot->pointers[1] = newLeaf;

        // Update new root's variables.
        newRoot->isLeaf = false;
        newRoot->numKeys = 1;

        // Update the root node
        root = newRoot;
      }
      // If we are not at the root, we need to insert a new parent in the middle levels of the tree.
      else
      {
        insertInternal(internalKey, parent, newLeaf);
      }
    }
  }

  // update numnodes ---- jw: not sure if this is required
  //numNodes = index->getAllocated();
}

// Updates the parent node to point at both child nodes, and adds a parent node if needed.
// Takes the lower bound of the right child, and the main memory address of the parent and the new child,
// as well as disk address of parent and new child.
void BPlusTree::insertInternal(float key, Node *cursor, Node *child)
{

  if (cursor->numKeys < maxKeys)
  {
    // Iterate through the parent to see where to put in the lower bound key for the new child.
    int i = 0;
    while (key > cursor->keys[i] && i < cursor->numKeys)
    {
      i++;
    }

    // Now we have i, the index to insert the key in. Bubble swap all keys back to insert the new child's key.
    // We use numKeys as index since we are going to be inserting a new key.
    for (int j = cursor->numKeys; j > i; j--)
    {
      cursor->keys[j] = cursor->keys[j - 1];
    }

    // Shift all pointers one step right (right pointer of key points to lower bound of key).
    for (int j = cursor->numKeys + 1; j > i + 1; j--)
    {
      cursor->pointers[j] = cursor->pointers[j - 1];
    }

    // Add in new child's lower bound key and pointer to the parent.
    cursor->keys[i] = key;
    cursor->numKeys++;
    cursor->pointers[i + 1] = child;

  }
  // If parent node doesn't have space, we need to recursively split parent node and insert more parent nodes.
  else
  {
    // Make new internal node (split this parent node into two).
    // Note: We DO NOT add a new key, just a new pointer!
    Node *newInternal = new Node(maxKeys);

    // Same logic as above, keep a temp list of keys and pointers to insert into the split nodes.
    // Now, we have one extra pointer to keep track of (new child's pointer).
    float tempKeyList[maxKeys + 1];
    Node* tempPointerList[maxKeys + 2];

    // Copy all keys into a temp key list.
    // Note all keys are filled so we just copy till maxKeys.
    for (int i = 0; i < maxKeys; i++)
    {
      tempKeyList[i] = cursor->keys[i];
    }

    // Copy all pointers into a temp pointer list.
    // There is one more pointer than keys in the node so maxKeys + 1.
    for (int i = 0; i < maxKeys + 1; i++)
    {
      tempPointerList[i] = (Node*)cursor->pointers[i];
    }

    // Find index to insert key in temp key list.
    int i = 0;
    while (key > tempKeyList[i] && i < maxKeys)
    {
      i++;
    }

    // Swap all elements higher than index backwards to fit new key.
    int j;
    for (int j = maxKeys; j > i; j--)
    {
      tempKeyList[j] = tempKeyList[j - 1];
    }

    // Insert new key into array in the correct spot (sorted).
    tempKeyList[i] = key;

    // Move all pointers back to fit new child's pointer as well.
    //2????
    for (int j = maxKeys + 2; j > i + 1; j--)
    {
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
    for (int i = 0; i < cursor->numKeys; i++)
    {
      cursor->keys[i] = tempKeyList[i];
    }
    
    // Insert new keys into the new internal parent node.
    for (i = 0, j = cursor->numKeys + 1; i < newInternal->numKeys; i++, j++)
    {
      newInternal->keys[i] = tempKeyList[j];
    }

    // Insert pointers into the new internal parent node.
    for (i = 0, j = cursor->numKeys + 1; i < newInternal->numKeys + 1; i++, j++)
    {
      newInternal->pointers[i] = tempPointerList[j];
    }
    // assign the new child to the original parent
    cursor->pointers[cursor->numKeys] = child;


    // If current cursor is the root of the tree, we need to create a new root.
    if (cursor == root)
    {
      Node *newRoot = new Node(nodeSize);
      // Update newRoot to hold the children.
      // Take the rightmost key of the old parent to be the root.
      // Although we threw it away, we are still using it to denote the leftbound of the old child.
      newRoot->keys[0] = cursor->keys[cursor->numKeys];

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
    else
    {
      insertInternal(tempKeyList[cursor->numKeys], findParent(root, cursor), newInternal);
    }
  }
}

Node *BPlusTree::findParent(Node *cursor, Node *child) {
  Node *parent;
  if (cursor->isLeaf || ((Node*)cursor->pointers[0])->isLeaf) {
    return NULL;
  }
  for (int i = 0; i < cursor->numKeys + 1; i++) {
    if (cursor->pointers[i] == child) {
      parent = cursor;
      return parent;
    } else {
      parent = findParent((Node*)cursor->pointers[i], child);
      if (parent != NULL)
        return parent;
    }
  }
  return parent;
}

Node *BPlusTree::search(int x, bool flag, bool printer)
{
    //search logic
    if(root==NULL)
    {
        //empty
    }
    else
    {
        Node* cursor = root;
        //in the following while loop, cursor will travel to the leaf node possibly consisting the key
        while(cursor->isLeaf == false)
        {
            for(int i = 0; i < cursor->numKeys; i++)
            {
                if(x < cursor->keys[i])
                {
                    if (printer == true) {
                        for (int j = 0; j < cursor->numKeys; j++) {
                            cout << cursor->keys[j] << " ";
                        }
                        cout << "\n";
                    }
                    cursor = (Node*)cursor->pointers[i];
                    break;
                }
                if(i == cursor->numKeys - 1)
                {
                    if (printer == true) {
                        for (int j = 0; j < cursor->numKeys; j++) {
                            cout << cursor->keys[j] << " ";
                        }
                        cout << "\n";
                    }
                    cursor = (Node*)cursor->pointers[i+1];
                    break;
                }
            }
        }
        //in the following for loop, we search for the key if it exists
        if (printer == true) {
            for (int j = 0; j < cursor->numKeys; j++) {
                cout << cursor->keys[j] << " ";
            }
            cout << "\n";
        }
        for(int i = 0; i < cursor->numKeys; i++)
        {
            if(cursor->keys[i] == x)
            {
                //cout<<"Found\n";
                return cursor;
            }
        }
        //cout<<"Not found\n";
        return NULL;
    }
    
    return NULL;
}

void BPlusTree::displayTree(Node *cursor, std::vector<std::string> *s, int *level){
  if(cursor->isLeaf){
    string item;
    item.append("|");
    for (int i = 0; i < cursor->numKeys; i++) {
      item.append(to_string((int)cursor->keys[i]));
      item.append("|");
    }
    item.append(" ");
    *level = 1;
    if(s->size() < *level){
      s->push_back(item);
    }
    else{
      s->at((*level)-1).append(item);
    }
    return;
  }

  string item;
  for (int i = 0; i < cursor->numKeys + 1; i++) {
    displayTree((Node*)cursor->pointers[i],s,level);

  }
  item.append("|");
  for (int i = 0; i < cursor->numKeys; i++) {
    item.append(to_string((int)cursor->keys[i]));
    item.append("|");
  }
  item.append(" ");
  (*level)+=1;
  if(s->size() < *level){
    s->push_back(item);
  }
  else{
    s->at(*level-1).append(item);
  }
  return;

}

void BPlusTree::display() {
  Node* root = getRoot();
  vector<string> s;
  int level = 0;

  Node* cursor = root;

  displayTree(cursor, &s, &level);

  for(int i = s.size()-1; i>0; i--){
    int spaceCount = (s[0].size() - s[i].size())/2;
    for(spaceCount;spaceCount>0;spaceCount--){
      cout << " ";
    }
    cout << s[i] << "\n";
    
  }
  cout << s[0] ;

}

int main() {
  BPlusTree node;
  char a = 't';

  node.insert(&a,12);
  node.insert(&a,11);
  node.insert(&a,10);
  node.insert(&a,9);
  node.insert(&a,8);
  node.insert(&a,7);
  node.insert(&a,6);

  node.display();
}
