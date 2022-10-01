CC = g++
CFLAGS = -static-libstdc++ -static-libgcc -std=c++0x

all : project1

project1 : main.o memory.o bplustree.o
	$(CC) $(CFLAGS) -o project1 main.o memory.o bplustree.o

main.o : main.cpp 
	$(CC) $(CLAGS) -c main.cpp

memory.o : memory.cpp
	$(CC) $(CFLAGS) -c memory.cpp

bplustree.o : bplustree.cpp
	$(CC) $(CFLAGS) -c bplustree.cpp

clean :
	rm *.o
