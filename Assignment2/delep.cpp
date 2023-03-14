#include <bits/stdc++.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <sys/wait.h>

using namespace std;

// Function to match the file with the open files of the processes
void fmatch(char* file, vector<int> &openpid, vector<int> &lockpid)
{
    // Buffer to store the path of the file
    char buf[1024];
    DIR *dir = opendir("/proc");
    if(dir == NULL)
    {
        perror("Can't open /proc\n");
        return;
    }

    // Iterate over all the processes
    struct dirent *entry;
    while((entry = readdir(dir)) != NULL)
    {
        // Skip non-directories
        if(entry->d_type != DT_DIR) continue;

        // Skip non-numeric directories
        char *endptr;
        int pid = strtol(entry->d_name, &endptr, 10);
        if(*endptr != '\0') continue;

        // Open the /proc/PID/fdinfo directory
        char path[128];
        sprintf(path,"/proc/%d/fdinfo", pid);

        // Open the directory
        DIR *cdir = opendir(path);
        if(cdir == NULL)
        {
            cout << "Can't open /proc/PID/fdinfo" << endl;
            kill(getppid(), SIGKILL);
        }

        // Iterate over all the files in the directory
        struct dirent *centry;
        while((centry = readdir(cdir)) != NULL)
        {
            // Skip non-directories
            char *endptr;
            int fid = strtol(centry->d_name, &endptr, 10);
            if(*endptr != '\0') continue;

            // Open the /proc/PID/fd/FID symlink
            sprintf(path,"/proc/%d/fd/%d", pid, fid);
            int len = readlink(path, buf, sizeof(buf) - 1);
            if(len == -1) continue;
            buf[len] = '\0';
            if(strcmp(buf, file) != 0) continue;
            openpid.push_back(pid);

            // Open the /proc/PID/fdinfo directory
            sprintf(path,"/proc/%d/fdinfo/%d", pid, fid);
            FILE *file_info = fopen(path, "r");
            if(file_info == NULL) continue;

            char line[256];
            // Iterate over all the lines in the file
            while(fgets(line, sizeof(line), file_info) != NULL)
            {
                // Check if the file is locked
                if(strcmp(strtok(line, ":"), "lock") == 0) lockpid.push_back(pid);
            }
            // Close the file
            fclose(file_info);
        }
        // Close the directory
        closedir(cdir);
    }
    // Close the directory
    closedir(dir);
}

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        printf("Usage: sudo delep <absolute_file_path>\n");
        exit(0);
    }
    // Create a pipe
    int fd[2];
    pipe(fd);
    // Create a child process
    if(fork() == 0)
    {
        // Find the processes using the file
        vector<int> openpid, lockpid;
        // Call the function to match the file with the open files of the processes
        fmatch(argv[1], openpid, lockpid);
        int arr1[openpid.size() + 1], arr2[lockpid.size() + 1];
        // Store the process ids in the array
        for(int i = 0; i < openpid.size(); i++) arr1[i] = openpid[i];
        arr1[openpid.size()] = -1;
        // Store the process ids in the array
        for(int i = 0; i < lockpid.size(); i++) arr2[i] = lockpid[i];
        arr2[lockpid.size()] = -1;
        // Close the read end of the pipe
        close(fd[0]);
        close(1);
        // Duplicate the write end of the pipe
        dup(fd[1]);
        // Write the process ids to the pipe
        write(1, arr1, sizeof(arr1));
        write(1, arr2, sizeof(arr2));
        exit(0);
    }
    // Parent process
    else
    {
        // Wait for the child process to finish
        wait(NULL);
        // Close the write end of the pipe
        close(fd[1]);
        dup2(0, fd[1]);
        close(0);
        // Duplicate the read end of the pipe
        dup(fd[0]);
        // Read the process ids from the pipe
        vector<int> openpid, lockpid;
        int arr[1000], change = 0;
        int n = read(fd[0], arr, sizeof(arr));
        // Store the process ids in the vector of open mode and lock mode
        for(int i = 0; i < n / 4; i++)
        {
            if(arr[i] == -1)
            {
                change = 1;
                continue;
            }
            if(change == 0) openpid.push_back(arr[i]);
            else lockpid.push_back(arr[i]);
        }
        // Close the read end of the pipe
        close(0);
        dup2(fd[1], 0);
        // Print the process ids
        printf("\nProcesses using this file in open mode ::\n");
        for(auto u: openpid) printf("%d ", u);
        printf("\n\nProcesses using this file in lock mode ::\n");
        for(auto u: lockpid) printf("%d ", u);
        printf("\n\nDo you want to kill these processes? (yes/no) : ");
        char prompt[4], c;
        // Take the input from the user
        scanf("%[^\n]s", prompt);
        scanf("%c", &c);
        // If the user wants to kill the processes
        if(strcmp(prompt, "yes") == 0)
        {
            for(auto u: openpid)
            {
                // Kill the process using the kill system call
                if(kill(u, SIGKILL) == -1)
                {
                    printf("\nError killing process %d\n", u);
                    return 1;
                }
            }
            printf("All processes which opened the file are killed\n");
            // Delete the file
            if(remove(argv[1]) == 0) printf("File deleted successfully\n");
            else printf("Unable to delete the file\n");
        }
    }
    return 0;
}
