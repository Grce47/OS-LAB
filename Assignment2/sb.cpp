#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>

using namespace std;

// Function to get the parent process of the given process
void p_tree(map<int, vector<int>> &ptree, map<int, int> &par)
{
    // Open the /proc directory
    DIR *dir = opendir("/proc");
    if(dir == NULL)
    {
        perror("Can't be open the /proc file\n");
        return;
    }

    struct dirent *entry;
    // Iterate over all the processes
    while((entry = readdir(dir)) != NULL)
    {
        if(entry->d_type != DT_DIR) continue;

        char *endptr;
        // convert the directory name to a number
        int pid = strtol(entry->d_name, &endptr, 10);
        if(*endptr != '\0') continue;

        char path[32];
        sprintf(path,"/proc/%d/status", pid);

        FILE *status_file = fopen(path, "r");
        if(status_file == NULL) continue;

        char line[256];
        // Read the status file line by line
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
    closedir(dir); // Close the /proc directory
}

// Function to get the number of processes in the process tree
void dfs(map<int, vector<int>> &ptree, int pid, int &cnt)
{
    cnt++;
    // If the process has child processes
    if(ptree.find(pid) != ptree.end())
    {
        for(auto u: ptree[pid]) dfs(ptree, u, cnt);
    }
}

int main(int argc, char** argv)
{
    // Check if the process id is given
    if(argc < 2)
    {
        printf("Usage: sb <pid> [-suggest]\n");
        return 0;
    }
    int pid = -1, ppid = -1;
    // Convert the process id to integer
    pid = atoi(argv[1]);
    // Create vector to store the process tree
    vector<int> plist;

    // Create map to store the process tree
    map<int, vector<int>> ptree;
    // Create map to store the parent process of each process
    map<int, int> par;

    // Get the process tree
    p_tree(ptree, par);
    
    // Print the process tree
    while(pid > 1)
    {
        plist.push_back(pid);
        ppid = par[pid];
        printf("Child process = %d  Parent process = %d\n", pid, ppid);
        pid = ppid;
    }
    plist.push_back(pid);
    printf("\n");

    // If the -suggest option is given
    if(argc == 3 && strcmp(argv[2], "-suggest") == 0)
    {
        // Get the number of processes in the process tree
        int itr = 2;
        vector<vector<int>> childCnt(itr);
        vector<int> pidChildCnt;
        // Iterate over the process tree
        for(int i = 0; i < itr; i++)
        {
            // For each process in the process tree
            for(auto u: plist)
            {
                // Get the number of processes in the process tree
                int cnt = 0;
                dfs(ptree, u, cnt);
                // Print the process id, number of processes in the process tree and number of child processes
                printf("%d :: %d ::: %d\n", u, cnt, (int)ptree[u].size());
                childCnt[i].push_back(cnt);
                // If the process is the root process
                if(u == plist[0]) pidChildCnt.push_back(ptree[u].size());
            }
            printf("\n");

            // Clear the process tree
            ptree.clear();
            par.clear();

            if(i == itr -  1) break;

            // Sleep for 2 minutes
            sleep(120);
            // Rebuild the process tree
            p_tree(ptree, par);
        }

        // Create vector to store the heuristic value
        vector<double> heu;
        // Calculate the heuristic value
        heu.push_back((pidChildCnt.back() - pidChildCnt[0]) * 0.9 / itr);
        // For each process in the process tree
        for(int i = 1; i < childCnt[0].size(); i++)
            heu.push_back((childCnt[itr - 1][i] * childCnt[0][i - 1]) / (double)(childCnt[itr - 1][i - 1] * childCnt[0][i]));

        // if the heuristic value is greater than 1.1, then the process is a malware
        for(int i = heu.size() - 1; i >= 0; i--)
            // If the heuristic value is greater than 1.1
            if(heu[i] > 1.1)
            {
                printf("Possible malware process is %d\n\n", plist[i]);
                return 0;
            }
        // If no malware is found
        printf("No malwares found\n\n");
    }
    return 0;
}
