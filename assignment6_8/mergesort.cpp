#include "goodmalloc.hpp"

void merge(string name, int start, int mid, int end, bool _freeElem)
{
    // Find the list
    int left_offset = start;
    int right_offset = mid + 1;
    // Create a temp element list to store the sorted values
    createList(end - start + 1, "temp");
    int i = 0;
    while (left_offset <= mid && right_offset <= end)
    {
        if (getVal(name, left_offset) < getVal(name, right_offset))
        {
            assignVal("temp", i, getVal(name, left_offset));
            left_offset++;
        }
        else
        {
            assignVal("temp", i, getVal(name, right_offset));
            right_offset++;
        }
        i++;
    }
    while (left_offset <= mid)
    {
        assignVal("temp", i, getVal(name, left_offset));
        left_offset++;
        i++;
    }
    while (right_offset <= end)
    {
        assignVal("temp", i, getVal(name, right_offset));
        right_offset++;
        i++;
    }
    for (int i = 0; i < end - start + 1; i++)
    {
        int val = getVal("temp", i);
        assignVal(name, start + i, val);
    }
    if (_freeElem)
        freeElem("temp");
}

void mergeSort(string name, int start, int end, bool _freeElem)
{
    if (start < end)
    {
        int mid = (start + end) / 2;
        mergeSort(name, start, mid, _freeElem);
        mergeSort(name, mid + 1, end, _freeElem);
        merge(name, start, mid, end, _freeElem);
    }
}

int main()
{
    printf("Size of element: %d\n", sizeof(element));
    // Create main memory of 250 MB
    int mainMemSize = 250 * 1024 * 1024;
    void *mainMem = createMem(mainMemSize);

    // Create list of 50000 elements
    int num_elements = 50000;
    createList(num_elements, "merge_sort");
    // Fill the values randomly from 1 to 1000000
    srand(time(NULL));
    for (int i = 0; i < num_elements; i++)
    {
        assignVal("merge_sort", i, rand() % 1000000 + 1);
    }

    // Print the list
    printList("merge_sort");
    mergeSort("merge_sort", 0, num_elements - 1, true);
    printList("merge_sort");
    freeElem("merge_sort");
    freeElem(""); // Clear all lists

    int num_runs_true = 100;
    // Get running time and memory footprint with freeElem = true
    auto start = chrono::high_resolution_clock::now();
    int total_mem_true = 0;
    for (int run = 0; run < num_runs_true; run++)
    {
        createList(num_elements, "merge_sort");
        // Fill the values randomly from 1 to 1000000
        srand(time(NULL));
        for (int i = 0; i < num_elements; i++)
        {
            assignVal("merge_sort", i, rand() % 1000000 + 1);
        }

        // Recursive merge sort
        mergeSort("merge_sort", 0, num_elements - 1, true);
        freeElem("merge_sort");

        // Get memory footprint
        total_mem_true += memoryFootprint();
    }
    auto end = chrono::high_resolution_clock::now();
    auto total_time_true = chrono::duration_cast<chrono::seconds>(end - start).count();

    // Get running time and memory footprint with freeElem = false
    int total_mem_false = 0;
    int num_runs_false = 1;
    auto started = std::chrono::high_resolution_clock::now();
    for (int run = 0; run < num_runs_false; run++)
    {
        createList(num_elements, "merge_sort");
        // Fill the values randomly from 1 to 1000000
        srand(time(NULL));
        for (int i = 0; i < num_elements; i++)
        {
            assignVal("merge_sort", i, rand() % 1000000 + 1);
        }

        // Recursive merge sort
        mergeSort("merge_sort", 0, num_elements - 1, false);

        // Get memory footprint
        total_mem_false += memoryFootprint();

        // Before the next run clear the list
        freeElem(""); // Clear all lists
    }
    auto done = std::chrono::high_resolution_clock::now();
    auto total_time_false = std::chrono::duration_cast<std::chrono::seconds>(done - started).count();

    cout << "With freeElem = true" << endl;
    cout << "Average running time: " << total_time_true / num_runs_true << " seconds" << endl;
    cout << "Average memory footprint: " << total_mem_true / num_runs_true << " bytes" << endl;

    cout << "With freeElem = false" << endl;
    cout << "Average running time: " << total_time_false / num_runs_false << " seconds" << endl;
    cout << "Average memory footprint: " << total_mem_false / num_runs_false << " bytes" << endl;

    return 0;
}