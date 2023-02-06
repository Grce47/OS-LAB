#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>

using namespace std;

map<int, vector<int>> ptree;
map<int, int> par;

void p_tree()
{
    DIR *dir = opendir("/proc");
    if(dir == NULL)
    {
        perror("Can't be open the /proc file\n");
        return;
    }

    struct dirent *entry;
    while((entry = readdir(dir)) != NULL)
    {
        if(entry->d_type != DT_DIR) continue;

        char *endptr;
        int pid = strtol(entry->d_name, &endptr, 10);
        if(*endptr != '\0') continue;

        char path[32];
        sprintf(path,"/proc/%d/status", pid);

        FILE *status_file = fopen(path, "r");
        if(status_file == NULL) continue;

        char line[256];
        while(fgets(line, sizeof(line), status_file) != NULL)
        {
            if(strncmp(line, "PPid:", 5) == 0)
            {
                int ppid;
                sscanf(line + 5, "%d", &ppid);
                ptree[ppid].push_back(pid);
                par[pid] = ppid;
                break;
            }
        }

        fclose(status_file);
    }

    closedir(dir);
}

void dfs(map<int, vector<int>> &ptree, int pid, int &cnt)
{
    cnt++;
    if(ptree.find(pid) != ptree.end())
    {
        for(auto u: ptree[pid]) dfs(ptree, u, cnt);
    }
}

int main(int argc, char** argv)
{
    int pid = -1, ppid = -1;
    pid = atoi(argv[1]);
    vector<int> plist;

    p_tree();

    while(pid > 1)
    {
        plist.push_back(pid);
        ppid = par[pid];
        printf("Child process = %d  Parent process = %d\n", pid, ppid);
        pid = ppid;
    }
    plist.push_back(pid);

    printf("\n");

    for(auto u: plist)
    {
        int cnt = 0;
        // dfs(ptree, u, cnt);
        // cnt = ptree[u].size();
        printf("%d :: %d\n", u, cnt);
    }

    return 0;
}