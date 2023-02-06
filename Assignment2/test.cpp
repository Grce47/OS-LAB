#include <bits/stdc++.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

using namespace std;

void fmatch(char* file, vector<int> &openpid, vector<int> &lockpid)
{
    char buf[1024];
    DIR *dir = opendir("/proc");
    if(dir == NULL)
    {
        perror("Can't open /proc\n");
        return;
    }

    struct dirent *entry;
    while((entry = readdir(dir)) != NULL)
    {
        if(entry->d_type != DT_DIR) continue;

        char *endptr;
        int pid = strtol(entry->d_name, &endptr, 10);
        if(*endptr != '\0') continue;
        if(pid == 9808 || pid == 9809) printf("%d\n",pid);

        char path[32];
        sprintf(path,"/proc/%d/fdinfo", pid);

        DIR *cdir = opendir(path);
        if(cdir == NULL)
        {
            perror("Can't open /proc/PID/fdinfo\n");
            return;
        }

        struct dirent *centry;
        while((centry = readdir(dir)) != NULL)
        {
            if(centry->d_type != DT_DIR) continue;

            char *endptr;
            int fid = strtol(centry->d_name, &endptr, 10);
            if(*endptr != '\0') continue;

            sprintf(path,"/proc/%d/fd/%d", pid, fid);
            int len = readlink(path, buf, sizeof(buf) - 1);
            if(len == -1) continue;
            buf[len] = '\0';
            if(strcmp(buf, file) != 0) continue;
            openpid.push_back(pid);

            
            sprintf(path,"/proc/%d/fdinfo/%d", pid, fid);
            FILE *file_info = fopen(path, "r");
            if(file_info == NULL) continue;

            char line[256];
            while(fgets(line, sizeof(line), file_info) != NULL)
            {
                if(strcmp(strtok(line, ":"), "lock") == 0) lockpid.push_back(pid);
            }

            fclose(file_info);
        }

        closedir(cdir);
    }

    closedir(dir);
}

int main(int argc, char *argv[])
{
    vector<int> openpid, lockpid;
    fmatch(argv[1], openpid, lockpid);
    printf("open ::\n");
    for(auto u: openpid) printf("%d ", u);
    printf("\nlock ::\n");
    for(auto u: lockpid) printf("%d ", u);
    return 0;
}
