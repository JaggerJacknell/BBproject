#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX 200

// List of allowed commands
char *allowedCommands[MAX];

// Entered command line
char cmdLine[1000];

// list of words in entered command line
// (cmdElems[0] contains the command and cmdElems[>0] contains arguments of the command)
char *cmdElems[MAX];

// root working directory of the Game : we can not leave this directory
char rwd[1024];

// variable to check if current command is a redirection
int isRedirectionCmd;

// File result of redirection
char *redirectionFile;

// Check if a folder exists
// This check is used before creating the Game working directory
// to avoid the creation of an already created directory
// If folder exist return 1
// Else return 0
int folderExists(char *ptrFile) {
    struct stat s;
    return (stat(ptrFile, &s) == 0);
}

// This function is used to wait for user command input
// Readline library is used
void getCommand(void) {
    // Wait for user input command
    char* lineRead = readline("\n[LEASH]»");

    // Check that command is not empty
    if (lineRead && *lineRead) {
        // Add input command to commands history
        add_history(lineRead);
    }

    // input command is copied to cmdLine variable
    strcpy(cmdLine, lineRead);
}

// Check if entered command is allowed
int existCommand(void) {

    // Check that command is not empty
    if (cmdElems[0] == NULL) {
        return 0;
    }

    // Loop on allowed commands and compare with the entered one.
    int j = 0;
    for (j = 0; j < MAX - 1; j++) {
        if (allowedCommands[j] == '\0') {
            break; // Last command is reached, stop process.
        }

        // If entered command is allowed the return 1.
        if (strcmp(cmdElems[0], allowedCommands[j]) == 0) {
            return 1;
        }
    }

    // Entered command is not found on allowed list
    return 0;
}

// Check if entered command is exit
// If yes return 1
// Else return 0
int isExitCommand(void) {
    if (strcmp(cmdElems[0], "exit") == 0) {
        return 1;
    }
    return 0;
}

// Check if entered command is pwd
// If yes return 1
// Else return 0
int isPwdCommand(void) {
    if (strcmp(cmdElems[0], "pwd") == 0) {
        return 1;
    }
    return 0;
}

// Check if entered command is cd
// If yes return 1
// Else return 0
int isCdCommand(void) {
    if (strcmp(cmdElems[0], "cd") == 0) {
        return 1;
    }
    return 0;
}

// Separate entered command line into command and arguments
void separateCommand(void) {

    // initialize redirection variables
    redirectionFile = NULL;
    isRedirectionCmd = 0;
    // Check if command line is a redirection
    if (strstr(cmdLine, ">") != NULL) {
        isRedirectionCmd = 1;

        char *cmdParts[20];
        char *currPart;
        int i = 0;

        // get the first element of the path
        currPart = strtok(cmdLine, ">");

        cmdParts[i] = currPart;

        // Loop on command elements : this is used to get command and file of redirection
        while (currPart != NULL) {

            i++;
            currPart = strtok(NULL, ">");
            cmdParts[i] = currPart;
        }
        cmdParts[i + 1] = NULL;

        if (i != 2) {
            printf("Invalid Command!");
            cmdElems[0] = NULL;
            return;
        }

        strcpy(cmdLine, cmdParts[0]);
        //cmdLine = cmdParts[0];
        redirectionFile = cmdParts[1];
    }

    // Pointer to the words of entered command line
    char* bufferPointer = cmdLine;

    // loop until max of command elements is reached
    int i;
    for (i = 0; i <= MAX; i++) {
        // loop on chars of entered command line until a non space is reached
        while (*bufferPointer && isspace(*bufferPointer)) {
            bufferPointer++; // move to next position
        }

        // Check made to stop the loop when end of entered command line is reached
        if (!*bufferPointer) {
            break;
        }

        // Save the pointer position of the found word on the commands elements
        cmdElems[i] = bufferPointer;

        // Loop on chars to detect the end of the word
        while (*bufferPointer && !isspace(*bufferPointer)) {
            bufferPointer++; // move to next position
        }

        // if empty then end of word is reached , and then \0 is set.
        // Then continue searching for other words contained on entered command line
        if (*bufferPointer) {
            *bufferPointer = 0; // Set end of word
            bufferPointer++; // Continue to next words
        }
    }

    // Set NULL value to determine then end of command elements
    cmdElems[i] = NULL;
}

// Wait for child process to end and suspend execution of current process
// pid is the chikd process id to wait for
void waitForPid(pid_t pid) {
    int status;
    while (waitpid(pid, &status, 0) < 0) {
        continue;
    }
}

//Handle Ctrl+C signal
void sigintHandler(int sig_num) {
    printf("error");
    fflush(stdout);
    exit(EXIT_SUCCESS);
}

//Handle Ctrl+C signal on parent process
void sigintParentHandler(int sig_num) {
    // remove current signal handler
    signal(SIGINT, SIG_DFL);
}

// Execute the entered command in a separate process
// result is the expected value to win the game
void execCommand(char* result) {
    // child process id
    pid_t pid;
    // execution result of the command
    char cmdOut[2048];
    // In and out file descriptors of the pipe
    int fd[2];
    // Pipe creation for communication from the child process to the parent
    pipe(fd);

    // Check that command is not empty
    if (cmdElems[0] == NULL) {
        return;
    }

    // for current process
    pid = fork();

    // If current process is the child one
    if (pid == 0) {
        // Register signal on child process
        signal(SIGINT, sigintHandler);

        close(fd[0]);
        // link stdout to the pipe
        dup2(fd[1], fileno(stdout));
        // As stderr is not linked to the pipe, error messages are shown by the child process
        close(fd[1]);
        // execute entered command line
        int i = execvp(cmdElems[0], cmdElems);
        // if error occurred while executing the command
        if (i < 0) {
            close(fd[1]);
            // Send error text to parent process
            printf("error");
            exit(EXIT_SUCCESS); // Exit child process
        }
        exit(EXIT_SUCCESS); // Exit child process
    } else { // Current process is the parent one

        // Register signal on child process
        signal(SIGINT, sigintParentHandler);

        close(fd[1]);

        // file of redirection command
        FILE *f;
        // if current command is a redirection then open file
        if (isRedirectionCmd == 1) {
            f = fopen(redirectionFile, "w");
            if (f == NULL) {
                printf("Error opening file!\n");
                return;
            }
        }

        // Clear content of cmdOut
        memset(&cmdOut[0], 0, sizeof(cmdOut));
        // Read messages from the child process
        while (read(fd[0], cmdOut, 2048) > 0) {
            // if message "error" the error message is shown by the child process so we nothing to do
            if (strcmp(cmdOut, "error") == 0) {
                return;
            }

            // If current command is a redirection then redirect output to file
            if (isRedirectionCmd == 1) {
                fprintf(f, "%s", cmdOut);
            } else {
                // print result of command line
                printf("%s", cmdOut);

                // check if command line output is the expected result to win the game
                if (strcmp(cmdOut, result) == 0) {
                    printf("\n\nBravo! level ok.\n\n");
                    exit(EXIT_SUCCESS);
                }
            }
        }

        if (f != NULL) {
            fclose(f);
        }

        // Wait for child process
        waitForPid(pid);
    }
}

// Change current working directory to dir
int changeDir(char *dir) {
    return chdir(dir);
}

// Handle the pwd command
void handlePwdCommand(void) {
    char cwd[1024];
    // get the current working directory
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        printf("Error while getting cwd");
    }
}

// Get the real path of entered one:
// if path contains ../ the path is translated the real path
// This function is used to check if path is inside the game root directory
char* getRealPath(char* lastDir) {
    // child process id
    pid_t pid;
    // execution result of the command
    char cmdOut[2048];
    // In and out file descriptors of the pipe
    int fd[2];
    // Pipe creation for communication from the child process to the parent
    pipe(fd);

    // for current process
    pid = fork();

    // If current process is the child one
    if (pid == 0) {

        close(fd[0]);
        // link stdout to the pipe
        dup2(fd[1], fileno(stdout));

        close(fd[1]);

        // Change working directory of child process
        changeDir(lastDir);

        // Get again the working directory : that translates entered path with .. to a real path
        char newChangedDir[2048];
        getcwd(newChangedDir, sizeof(newChangedDir));
        printf("%s", newChangedDir);

        exit(EXIT_SUCCESS);
    } else { // Current process is the parent one
        close(fd[1]);

        memset(&cmdOut[0], 0, sizeof(cmdOut));
        read(fd[0], cmdOut, 2048);

        waitpid(pid, 0, 0);
        return cmdOut;
    }
}

// Check if directory is valid : exists and rights ok
int isValidDirectory(char* dir) {
    struct stat fileStat;
    if (stat(dir, &fileStat) >= 0) {
        if (S_ISDIR(fileStat.st_mode) && (fileStat.st_mode & S_IWUSR)
                && (fileStat.st_mode & S_IXUSR)) {
            return 1;
        }
    }
    return 0;
}

// check that first contains second text
// this function is used to check that we did not leave game root directory
int startsWith(const char *first, const char *second) {
    size_t firstSize, secondSize;

    firstSize = strlen(first);
    secondSize = strlen(second);

    if (secondSize <= firstSize && strncmp(first, second, secondSize) == 0) {
        return 1;
    }
    return 0;
}

// Handle the cd command
void handleCdCommand(void) {
    // Get the current working directory
    char lastDir[1024];
    getcwd(lastDir, sizeof(lastDir));

    // If no path is entered after the cd command then move to game's root directory
    if (cmdElems[1] == NULL) {
        changeDir(rwd);
    } else {

        char *currDir;

        // get the first element of the path
        currDir = strtok(cmdElems[1], "/");

        char* lastDirPtr = &lastDir[0];

        // Loop on directory elements : this is used to check all crossed directories
        while (currDir != NULL) {

            strcat(lastDirPtr, "/");
            strcat(lastDirPtr, currDir);

            // Check that directory is valid
            if (isValidDirectory(lastDirPtr) != 1) {
                printf("No access rights or folder does not exist :\n %s\n",
                        lastDirPtr);
                return;
            }

            // get path without ..
            lastDirPtr = getRealPath(lastDirPtr);

            // Check that we did not leave the root directory of the game
            if (startsWith(lastDirPtr, rwd) != 1) {
                printf("You can not leave the root directory!\n");
                return;
            }

            currDir = strtok(NULL, "/");
        }
        // If all path elements are ok the change current working directory
        changeDir(lastDirPtr);
    }
}

// Show instructions
void showInstructions(void) {
    printf("The only authorized commands are:\n");
    int i;
    for (i = 0; allowedCommands[i] != '\0'; i++) {
        printf("%d : %s   \n", i, allowedCommands[i]);
    }
}

int main(int argc, char *argv[]) {

    int i = 0;
    // commands used to extract content of archive file
    char* args[7];
    args[0] = "tar";
    args[1] = "xzvf";
    args[2] = argv[1]; // the argument of program is the archive file name
    args[3] = "-C";
    args[4] = "./lol";
    args[5] = NULL;

    // file used to read content of meta file
    FILE *fp;
    // line of meta file
    char *line = NULL;
    // result expected to win the game
    char *result = NULL;

    size_t len = 10;
    ssize_t read;
    int result_code;

    // check that game root does not exist
    if (!(folderExists("lol"))) {

        mode_t mask = umask(0); // set umask to 0 (no restrictions)
        result_code = mkdir("lol", 0700); // Create root directory of the game
        umask(mask); // Reset the umask
    } else {
        result_code = 0;
    }

    // Check result code of directory creation
    switch (result_code) {

    // error creating the directory
    case -1:

        perror(argv[0]);
        printf("Error creating the root directory\n");
        exit(EXIT_FAILURE);
        break;

        // creation ok
    case 0:

        printf("Directory successfully created \n");

        // id of the child directory used to extract content of archive file
        pid_t pid;

        // fork current process
        if ((pid = fork()) < 0) { // Error to fork process
            printf("Error while creating fork of current process\n");
            exit(EXIT_FAILURE);
        } else if (pid == 0) { // current process is the child one
            // extract content of the archive file
            if (execvp(*args, args) < 0) { // if error occurred
                printf("Error extracting archive file\n");
                exit(EXIT_FAILURE);
            }

        } else {
            // Wait for child process to terminate
            waitForPid(pid);
        }

        // Change working directory to the root directory of the game
        if (changeDir("./lol/") != 0) {
            printf("Error to change working directory \n");
            exit(EXIT_FAILURE);
            break;
        }

        // Save current working directory to rwd
        getcwd(rwd, sizeof(rwd));

        fp = fopen("./meta", "r"); //open file for reading
        if (fp == NULL) {
            fprintf(stderr, "Error opening file \n");
            exit(EXIT_FAILURE);
        }

        // Read lines of meta file
        while ((read = getline(&line, &len, fp)) != -1) {
            // If line starts with # then ignore it and pass to next line
            if (strchr(line, '#') != NULL) {
                continue;
            }
            // Line starts with $ means that it's an allowed command
            else if (strchr(line, '$') != NULL) {
                allowedCommands[i] = strdup(line + 2);
                allowedCommands[i] = strtok(allowedCommands[i], "\n");
                i++;
            }
            // Line starts with > means that it's the expected result to win
            else if (strchr(line, '>') != NULL) {
                result = strdup(line + 2);
            }

        }

        // Remove meta file
        unlink("./meta");
        fclose(fp);
        free(line);

        break;

    default:
        break;

    }

    while (1) {
        // Read user input command
        getCommand();
        // Separate entered line to command and arguments
        separateCommand();
        //If command is empty
        if (cmdElems[0] == NULL) {
            //show command instructions
            showInstructions();
            continue;
        }

        // Check is entered command is pwd
        if (isPwdCommand() == 1) {
            handlePwdCommand();
            continue;
        }

        // Check is entered command is cd
        if (isCdCommand() == 1) {
            handleCdCommand();
            continue;
        }

        // Check is entered command is exit
        if (isExitCommand() == 1) {
            exit(EXIT_SUCCESS);
        }

        // Check is command is allowed if yes process it, else show instructions
        if (existCommand() == 1) {
            execCommand(result);
            signal(SIGINT, SIG_DFL);
        } else {
            showInstructions();
        }

    }
    return 0;
}
