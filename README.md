# CZ4031-Project-1 (Group 12)

## Description
The aim of this project is to design and implement the following two components of a database management system, storage and indexing. Our choice of programming language is C++.

## Overview
Our project consists of 3 main components working together:
- Storage - Design of the storage component initializing how each data item is stored as a field, how fields are packed into a record, and how records are packed into a block.
- B+ Tree - Design of our B+ tree structure but search, insert and delete functions.
- Experiments - Results of the experiments specified in Task 2.

## Instructions
a) Clone the [repository](https://github.com/fabecode/CZ4031-Project-1.git) from Github or download the zip file

b) Install a compiler of your choice (Ensure that the latest version is downloaded and supports C++ 11 and above) 
- [MingW](https://www.mingw-w64.org/downloads/)
- [Clang](https://clang.llvm.org/get_started.html)

c) Open Visual Studio Code or any preferred IDE

d) Run build task on [main.cpp](https://github.com/fabecode/CZ4031-Project-1/blob/main/main.cpp) using Clang or MingW compiler (ensure that data.tsv is also in the same folder)
Do note that the json configuration files in our github repository are configured for Clang.

e) Running the code will output the results into two different text files, [results_200B.txt](https://github.com/fabecode/CZ4031-Project-1/blob/main/results_200B.txt) and [results_500B.txt](https://github.com/fabecode/CZ4031-Project-1/blob/main/results_500B.txt) for experiments using 200B and 500B blocksize respectively. 
