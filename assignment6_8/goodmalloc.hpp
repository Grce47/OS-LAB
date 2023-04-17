#ifndef _GOODMALLOC
#define _GOODMALLOC

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <list>
#include <stack>
#include <set>
#include <chrono>

using namespace std;

typedef struct element
{
    int val;
    struct element *next;
    struct element *prev;
} element;

typedef struct block
{
    string name;   // Name of the list
    int size;      // Size of the block
    int free;      // 1 if free, 0 if not
    void *pointer; // Pointer to the memory
} block;



// A page table to store the mapping from list name to virtual address and physical address
typedef struct pageTable
{
    void *physicalAddr; // Physical address of the list
    int numElements;    // Number of elements in the list
} pageTable;



void printBlockList();

void printList(string);

void *createMem(int);

void createList(int, string);

void assignVal(string, int, int);

int getVal(string, int);

void freeElem(string);

void startScope();

void endScope();

int memoryFootprint();

#endif