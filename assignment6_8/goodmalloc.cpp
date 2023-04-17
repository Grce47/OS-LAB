#include "goodmalloc.hpp"

void *memStart; // Pointer to the start of the memory
int memSize;    // Size of the memory

// Stack of vector of string
stack<set<string>> scopeStack;
bool SCOPE_FLAG = false;

// Page table entries
map<string, stack<pageTable>> pageTableEntries;

// STL list to store the blocks
list<block> blocks;

void printBlockList()
{
    cout << "Block list: " << endl;
    for (list<block>::iterator it = blocks.begin(); it != blocks.end(); it++)
    {
        cout << "Name: " << it->name << ", Size: " << it->size << ", Free: " << it->free << ", Pointer: " << it->pointer << endl;
    }
}

void printList(string name)
{
    // Check if page table entry exists
    if (pageTableEntries.find(name) == pageTableEntries.end())
    {
        cerr << "Error: Page table entry for " << name << " does not exist." << endl;
        exit(1);
    }

    // Find the element
    element *list = (element *)pageTableEntries[name].top().physicalAddr;
    element *currElement = list;
    while (currElement != NULL)
    {
        cout << currElement->val << " ";
        currElement = currElement->next;
    }
    cout << endl;
}

void *createMem(int size)
{
    void *mem = malloc(size);
    if (mem == NULL)
    {
        printf("Error: malloc failed to allocate memory.\n");
        exit(1);
    }
    printf("Allocated %d bytes of memory at %p\n", size, mem);
    memStart = mem;
    memSize = size;

    // Create the head block
    block head = {"", size, 1, mem};
    blocks.push_back(head);

    return mem;
}

void createList(int num_elements, string name)
{
    int size = num_elements * sizeof(element);
    // Find a free block of memory
    block freeBlock;
    bool found = false;
    for (list<block>::iterator it = blocks.begin(); it != blocks.end(); it++)
    {
        if (it->free == 1 && it->size >= size)
        {
            freeBlock = *it;
            it->free = 0;
            it->name = name;
            it->size = size;
            found = true;
            break;
        }
    }

    if (!found)
    {
        cout << "Error: No free block of memory large enough to create list " << name << endl;
        exit(1);
    }
    cout << "Found free block of memory at " << freeBlock.pointer << " of size " << freeBlock.size << endl;

    if (freeBlock.size > size)
    {
        // Split the block
        block newBlock = {"", freeBlock.size - size, 1, (void *)((char *)freeBlock.pointer + size)};
        // Insert at appropriate position
        for (list<block>::iterator it = blocks.begin(); it != blocks.end(); it++)
        {
            // Insert after the current block
            if (it->pointer == freeBlock.pointer)
            {
                if (it != --blocks.end())
                    blocks.insert(it, newBlock);
                else
                    blocks.push_back(newBlock);
                break;
            }
        }
    }

    // Block list
    // printBlockList();

    cout << "Created list of name " << name << " of size " << num_elements << " at " << freeBlock.pointer << endl;

    // Create the list and initialize the values
    element *list = (element *)freeBlock.pointer;
    element *currElement = list;
    for (int i = 0; i < num_elements; i++)
    {
        currElement->val = i;
        if (i < num_elements - 1)
        {
            currElement->next = currElement + 1;
            currElement->next->prev = currElement;
        }
        else
        {
            currElement->next = NULL;
        }
        currElement++;
    }

    // Add the page table entry
    pageTable listPageTable;
    listPageTable.physicalAddr = freeBlock.pointer;
    listPageTable.numElements = num_elements;
    pageTableEntries[name].push(listPageTable);

    // Add the list name to the current scope
    if (SCOPE_FLAG)
        scopeStack.top().insert(name);
}

void assignVal(string name, int offset, int val)
{
    // Check if page table entry exists
    if (pageTableEntries.find(name) == pageTableEntries.end())
    {
        cerr << "Error: Page table entry for " << name << " does not exist." << endl;
        exit(1);
    }

    // Check if offset is valid
    if (offset >= pageTableEntries[name].top().numElements)
    {
        cerr << "Error: Offset " << offset << " is out of bounds for " << name << " list." << endl;
        exit(1);
    }

    // Find the element
    element *list = (element *)pageTableEntries[name].top().physicalAddr;
    element *element = list + offset;
    element->val = val;
}

int getVal(string name, int offset)
{
    // Check if page table entry exists
    if (pageTableEntries.find(name) == pageTableEntries.end())
    {
        cerr << "Error: Page table entry for " << name << " does not exist." << endl;
        exit(1);
    }

    // Check if offset is valid
    if (offset >= pageTableEntries[name].top().numElements)
    {
        cerr << "Error: Offset " << offset << " is out of bounds for " << name << " list." << endl;
        exit(1);
    }

    // Find the element
    element *list = (element *)pageTableEntries[name].top().physicalAddr;
    element *element = list + offset;
    return element->val;
}

void freeElem(string name = "")
{
    if (name == "")
    {
        // Clear all lists
        for (map<string, stack<pageTable>>::iterator it = pageTableEntries.begin(); it != pageTableEntries.end(); it++)
        {
            // For each name clear its stack
            for (stack<pageTable> pageTableStack = it->second; !pageTableStack.empty(); pageTableStack.pop())
            {
                freeElem(it->first);
            }
        }
    }
    else
    {
        // Get the page table entry
        pageTable pageTableEntry = pageTableEntries[name].top();

        // Find the block using the physical address
        list<block>::iterator to_find;
        bool found = false;
        for (list<block>::iterator it = blocks.begin(); it != blocks.end(); it++)
        {
            if (it->pointer == pageTableEntry.physicalAddr)
            {
                to_find = it;
                found = true;
                break;
            }
        }

        if (!found)
        {
            cout << "Error: Could not find block to free." << endl;
            exit(1);
        }

        // Merge with previous block if free
        if (to_find != blocks.begin())
        {
            list<block>::iterator prev = to_find;
            prev--;
            if (prev->free == 1)
            {
                prev->size += to_find->size;
                blocks.erase(to_find);
                to_find = prev;
            }
        }
        // Merge with next block if free
        if (to_find != --blocks.end())
        {
            list<block>::iterator next = to_find;
            next++;
            if (next->free == 1)
            {
                to_find->size += next->size;
                blocks.erase(next);
            }
        }

        // Mark the block as free
        to_find->free = 1;
        to_find->name = "";

        // Remove the page table entry
        pageTableEntries[name].pop();

    }
}

void startScope()
{
    // This function is called at the start of a function call
    // Create a new set of variables
    set<string> scope;
    // Push the new set onto the stack
    scopeStack.push(scope);
}

void endScope()
{
    // This function is called at the end of a function call
    // Pop the top set of variables off the stack

    for(auto it: scopeStack.top())
    {
        freeElem(it);
    }
    scopeStack.pop();
}

int memoryFootprint()
{
    int total = 0;
    for (list<block>::iterator it = blocks.begin(); it != blocks.end(); it++)
    {
        if (it->free == 0)
        {
            total += it->size;
        }
    }
    return total;
}