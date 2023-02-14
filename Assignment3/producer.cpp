#include <iostream>
#include <vector>
#include <unistd.h>
#include <algorithm>
#include <fstream>
using namespace std;

int rand_num(int l, int r)
{
    return l + (rand() % (r - l + 1));
}

vector<vector<int>> tree;
vector<int> nodes;

void load_init(const char *file_name)
{
    ifstream is(file_name);
    int x, y;
    while (is >> x >> y)
    {
        if (tree.size() <= max(x, y))
        {
            tree.resize(max(x, y) + 1);
        }
        tree[x].push_back(y);
        tree[y].push_back(x);
        nodes.push_back(x);
        nodes.push_back(y);
    }
    random_shuffle(nodes.begin(), nodes.end());
}

void add_node(int k)
{
    
}

int main()
{
    char const *file_name = "facebook_combined.txt";
    srand(time(0));
    load_init(file_name);

    while(1)
    {
        sleep(50);
        int m = rand_num(10,30);
        for(int i=0;i<m;i++) 
        {
            int k = rand_num(1,20);
            add_node(k);
        }
    }

    return 0;
}