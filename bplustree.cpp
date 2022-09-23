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

  numKeys = 0;
}

BPlusTree::BPlusTree()
{
  // change maxkeys!!!!!!!!!!!!
  maxKeys = 3;

  if (maxKeys == 0)
  {
    throw std::overflow_error("Overflow Error: Too much key & ptr in 1 node");
  }

  // Initialize node
  root = nullptr;


  degree = 0;
  numNodes = 0;

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
//  numNodes = index->Allocated();
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

void BPlusTree::remove(float key)
{
  // set numNodes before deletion
 // numNodes = index->getAllocated();

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
        if (key < cursor->keys[i])
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
    }

    // now that we have found the leaf node that might contain the key, we will try and find the position of the key here (if exists)
    // search if the key to be deleted exists in this bplustree
    bool found = false;
    int pos;
    // also works for duplicates
    for (pos = 0; pos < cursor->numKeys; pos++)
    {
      if (cursor->keys[pos] == key)
      {
        found = true;
        break;
      }
    }

    // If key to be deleted does not exist in the tree, return error.
    if (!found)
    {
      std::cout << "Can't find specified key " << key << " to delete!" << endl;
      
      // update numNodes and numNodesDeleted after deletion
      // int numNodesDeleted = numNodes - index->getAllocated();
      // numNodes = index->getAllocated();
      // return numNodesDeleted;
      return;
    }

    // pos is the position where we found the key.
    // We must delete the entire linked-list before we delete the key, otherwise we lose access to the linked list head.
    Node *temp = cursor;
    int temppos = pos;
    int keyshiftcount=0;
    int nodeshiftcount = 0;
    
    while(temp->keys[temppos] == key){
      keyshiftcount++;
      temppos ++;

      if(temppos == temp->numKeys || temp->keys[temppos] != key){
        for (int i = pos; i < temp->numKeys; i++)
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

        temp = (Node*)temp->pointers[temp->numKeys];
        temppos = 0;
        keyshiftcount = 0;
        //if(temp->keys[temppos]==key)
        nodeshiftcount++;
      }

    }
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
      
      // update numNodes and numNodesDeleted after deletion
      //int numNodesDeleted = numNodes - index->getAllocated();

      //return numNodesDeleted;
    }
    temp = cursor;
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
          parent->keys[leftSibling] = cursor->keys[0];

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

          // Update right sibling (shift keys and pointers left)
          for (int i = 0; i < rightNode->numKeys; i++)
          {
            rightNode->keys[i] = rightNode->keys[i + 1];
            rightNode->pointers[i] = rightNode->pointers[i + 1];
          }

          // Move right sibling's last pointer left by one too.
          rightNode->pointers[cursor->numKeys] = rightNode->pointers[cursor->numKeys + 1];

          // Update parent node's key to be new lower bound of right sibling.
          parent->keys[rightSibling - 1] = rightNode->keys[0];

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

        // We need to update the parent in order to fully remove the right node.
        removeInternal(parent->keys[rightSibling - 1], parent, rightNode);

        // Now that we have updated parent, we can just delete the right node from disk.
        delete[] cursor->keys;
        delete[] cursor->pointers;
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


// Takes in the parent disk address, the child address to delete, and removes the child.
void BPlusTree::removeInternal(float key, Node *cursor, Node *child)
{
  // If current parent is root
  if (cursor == root)
  {
    // If we have to remove all keys in root (as parent) we need to change the root to its child.
    if (cursor->numKeys == 1)
    {
      // If the larger pointer points to child, make it the new root.
      if (cursor->pointers[1] == child)
      {
        // Delete the child completely
        delete[] child->keys;
        delete[] child->pointers;
        delete child;

        // Set new root to be the parent's left pointer
        // Load left pointer into main memory and update root.
        root = (Node*)cursor->pointers[0];

        delete[] cursor->keys;
        delete[] cursor->pointers;
        delete cursor;

        // Nothing to save to disk. All updates happened in main memory.
        std::cout << "Root node changed." << endl;
        return;
      }
      // Else if left pointer in root (parent) contains the child, delete from there.
      else if (cursor->pointers[0] == child)
      {
        // Delete the child completely
        delete[] child->keys;
        delete[] child->pointers;
        delete child;
        

        // Set new root to be the parent's right pointer
        // Load right pointer into main memory and update root.
        root = (Node*)cursor->pointers[1];

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
  for (pos = 0; pos < cursor->numKeys; pos++)
  {
    if (cursor->keys[pos] == key)
    {
      break;
    }
  }

  // Delete the key by shifting all keys forward
  for (int i = pos; i < cursor->numKeys; i++)
  {
    cursor->keys[i] = cursor->keys[i + 1];
  }

  // Search for pointer to delete in parent
  // Remember pointers are on the RIGHT for non leaf nodes.
  for (pos = 0; pos < cursor->numKeys + 1; pos++)
  {
    if (cursor->pointers[pos] == child)
    {
      break;
    }
  }

  // Now move all pointers from that point on forward by one to delete it.
  for (int i = pos; i < cursor->numKeys + 1; i++)
  {
    cursor->pointers[i] = cursor->pointers[i + 1];
  }

  // Update numKeys
  cursor->numKeys--;

  // Check if there's underflow in parent
  // No underflow, life is good.
  if (cursor->numKeys >= (maxKeys + 1) / 2 - 1)
  {
    return;
  }

  // If we reach here, means there's underflow in parent's keys.
  // Try to steal some from neighbouring nodes.
  // If we are the root, underflow doesn't matter.
  if (cursor == root)
  {
    return;
  }

  // If not, we need to find the parent of this parent to get our siblings.
  // Pass in lower bound key of our child to search for it.
  Node *parent = findParent((Node *)root, cursor);
  int leftSibling, rightSibling;

  // Find left and right sibling of cursor, iterate through pointers.
  for (pos = 0; pos < parent->numKeys + 1; pos++)
  {
    if (parent->pointers[pos] == cursor)
    {
      leftSibling = pos - 1;
      rightSibling = pos + 1;
      break;
    }
  }

  // Try to borrow a key from either the left or right sibling.
  // Check if left sibling exists. If so, try to borrow.
  if (leftSibling >= 0)
  {
    // Load in left sibling from disk.
    
    Node *leftNode = (Node*)parent->pointers[leftSibling];

    // Check if we can steal (ahem, borrow) a key without underflow.
    // Non leaf nodes require a minimum of ⌊n/2⌋
    if (leftNode->numKeys >= (maxKeys + 1) / 2)
    {
      // We will insert this borrowed key into the leftmost of current node (smaller).
      // Shift all remaining keys and pointers back by one.
      for (int i = cursor->numKeys; i > 0; i--)
      {
        cursor->keys[i] = cursor->keys[i - 1];
      }

      // Transfer borrowed key and pointer to cursor from left node.
      // Basically duplicate cursor lower bound key to keep pointers correct.
      cursor->keys[0] = parent->keys[leftSibling];
      parent->keys[leftSibling] = leftNode->keys[leftNode->numKeys - 1];

      // Move all pointers back to fit new one
      for (int i = cursor->numKeys + 1; i > 0; i--)
      {
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
  if (rightSibling <= parent->numKeys)
  {
    // If we do, load in right sibling from disk.
    Node *rightNode = (Node*)parent->pointers[rightSibling];
    // Check if we can steal (ahem, borrow) a key without underflow.
    if (rightNode->numKeys >= (maxKeys + 1) / 2)
    {
      // No need to shift remaining pointers and keys since we are inserting on the rightmost.
      // Transfer borrowed key and pointer (leftmost of right node) over to rightmost of current node.
      cursor->keys[cursor->numKeys] = parent->keys[pos];
      parent->keys[pos] = rightNode->keys[0];

      // Update right sibling (shift keys and pointers left)
      for (int i = 0; i < rightNode->numKeys - 1; i++)
      {
        rightNode->keys[i] = rightNode->keys[i + 1];
      }

      // Transfer first pointer from right node to cursor
      cursor->pointers[cursor->numKeys + 1] = rightNode->pointers[0];

      // Shift pointers left for right node as well to delete first pointer
      for (int i = 0; i < rightNode->numKeys; ++i)
      {
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
  if (leftSibling >= 0)
  {
    // Load in left sibling from disk.
    Node *leftNode = (Node*)parent->pointers[leftSibling];

    // Make left node's upper bound to be cursor's lower bound.
    leftNode->keys[leftNode->numKeys] = parent->keys[leftSibling];

    // Transfer all keys from current node to left node.
    // Note: Merging will always suceed due to ⌊(n)/2⌋ (left) + ⌊(n-1)/2⌋ (current).
    for (int i = leftNode->numKeys + 1, j = 0; j < cursor->numKeys; j++)
    {
      leftNode->keys[i] = cursor->keys[j];
    }

    // Transfer all pointers too.
    for (int i = leftNode->numKeys + 1, j = 0; j < cursor->numKeys + 1; j++)
    {
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
  else if (rightSibling <= parent->numKeys)
  {
    // Load in right sibling from disk.
    Node *rightNode = (Node*)parent->pointers[rightSibling];

    // Set upper bound of cursor to be lower bound of right sibling.
    cursor->keys[cursor->numKeys] = parent->keys[rightSibling - 1];

    // Note we are moving right node's stuff into ours.
    // Transfer all keys from right node into current.
    // Note: Merging will always suceed due to ⌊(n)/2⌋ (left) + ⌊(n-1)/2⌋ (current).
    for (int i = cursor->numKeys + 1, j = 0; j < rightNode->numKeys; j++)
    {
      cursor->keys[i] = rightNode->keys[j];
    }

    // Transfer all pointers from right node into current.
    for (int i = cursor->numKeys + 1, j = 0; j < rightNode->numKeys + 1; j++)
    {
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

//int main() {
//  BPlusTree node;
//  char a = 't';

//  node.insert(&a,12);
//  node.insert(&a,11);
//  node.insert(&a,10);
//  node.insert(&a,10);
//  node.insert(&a,9);
//  node.insert(&a,8);
// node.insert(&a,7);
//  node.insert(&a,6);
//  node.remove(10);

//  node.display();
//}
