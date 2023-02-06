# Bash application using syscalls

### OS Group 8

- Grace Sharma
- Umang Singla
- Saurabh Das
- Mradul Agrawal

Compilation command:

```bash
    g++ Assignment2_8_20CS30034_20CS30051_20CS10068_20CS30022.cpp -std=c++17 -lstdc++fs -lreadline -o a.out
```

Execution command:
```
    ./a.out
```

1. Run an external command

2. Run an external command by redirecting standard input from a file

3. Run an external command by redirecting standard output to a file

4. Combination of input and output redirection

5. Run an external command in the background with possible input and output redirections

    - Default behaviour: Process execute in background with output in STDOUT but blocks on STDIN

6. Run several external commands in the pipe mode

7. Interrupting commands running in your shell (using signal call)

    - Control C with unexecuted commands: Done using readline functions

8. Implementing cd and pwd

    - cd and pwd implemented but some test cases needed to be checked for cd

9. Handling wildcards in commands (‘*’ and ‘?’)

10. Implementing searching through history using up/down arrow keys

11. Command to detect a simple malware

12. Command to check for file locks

13. Features to help editing commands
