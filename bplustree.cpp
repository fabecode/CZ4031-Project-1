#include "bplustree.h"
#include "memory.h"

#include <iostream>
#include <tuple>
#include <unordered_map>
#include <cstring>
#include <array>

using namespace std;

bool myNullPtr = false;

Node::Node(int maxKeys)
{
  // Initialize array with key and ptr
  keys = new float[maxKeys];
  pointers = new Address[maxKeys + 1];

  for (int i = 0; i < maxKeys + 1; i++)
  {
    Address nullAddress{(void *)myNullPtr, 0};
    pointers[i] = nullAddress;
  }
  numKeys = 0;
}

BPlusTree::BPlusTree(std::size_t blockSize, MemoryPool *disk, MemoryPool *index)
{

  size_t nodeBufferSize = blockSize - sizeof(bool) - sizeof(int);
  size_t sum = sizeof(Address);
  maxKeys = 0;

  // Max ptr and key pair
  while (sum + sizeof(Address) + sizeof(float) <= nodeBufferSize)
  {
    sum += (sizeof(Address) + sizeof(float));
    maxKeys += 1;
  }

  if (maxKeys == 0)
  {
    throw std::overflow_error("Overflow Error: Too much key & ptr in 1 node");
  }

  // Initialize node
  rootAddress = nullptr;
  root = nullptr;

  nodeSize = blockSize;

  degree = 0;
  numNodes = 0;

  // Initialize disk space 
  this->disk = disk;
  this->index = index;
}