#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>

using namespace std;

void p_tree(map<int, vector<int>> &ptree, map<int, int> &par)
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

    map<int, vector<int>> ptree;
    map<int, int> par;

    p_tree(ptree, par);

    while(pid > 1)
    {
        plist.push_back(pid);
        ppid = par[pid];
        printf("Child process = %d  Parent process = %d\n", pid, ppid);
        pid = ppid;
    }
    plist.push_back(pid);
    printf("\n");

    if(argc == 3 && strcmp(argv[2], "-suggest") == 0)
    {
        int itr = 2;
        vector<vector<int>> childCnt(itr);
        vector<int> pidChildCnt;
        for(int i = 0; i < itr; i++)
        {
            for(auto u: plist)
            {
                int cnt = 0;
                dfs(ptree, u, cnt);
                printf("%d :: %d ::: %d\n", u, cnt, ptree[u].size());
                childCnt[i].push_back(cnt);
                if(u == plist[0]) pidChildCnt.push_back(ptree[u].size());
            }
            printf("\n");

            ptree.clear();
            par.clear();

            if(i == itr -  1) break;

            sleep(20);
            p_tree(ptree, par);
        }

        vector<double> heu;
        heu.push_back((pidChildCnt.back() - pidChildCnt[0]) * 0.9 / itr);
        for(int i = 1; i < childCnt[0].size(); i++)
            heu.push_back((childCnt[itr - 1][i] * childCnt[0][i - 1]) / (double)(childCnt[itr - 1][i - 1] * childCnt[0][i]));

        for(int i = heu.size() - 1; i >= 0; i--)
            if(heu[i] > 1.1)
            {
                printf("Possible malware process is %d\n\n", plist[i]);
                return 0;
            }
        printf("No malwares found\n\n");
    }
    return 0;
}
