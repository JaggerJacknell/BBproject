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
char *strings[MAX]; //tableau dans lequel on va enregistrer les commandes possibles pour l'utilisateur

char cmdLine[1000];
char *cmdElems[MAX]; // pointe sur les mots d'une ligne de commande
int result_code;
char rwd[1024];

int folderExists(char *ptrFile) // ajout à tester pour éviter de créér le réprtoire lol deux fois!!!
{
// Renvoie 1 si existe, 0 sinon
    struct stat s;
    return (stat(ptrFile, &s) == 0);
}

void showPrompt() {
    //printf("\n[LEASH]»»");
    //fflush(stdout); // vider le buffer
}

void getCommand() {
    char* lineRead = readline("\n[LEASH]»");
    if (lineRead && *lineRead) {
        add_history(lineRead);
    }

    //fgets(cmdLine, sizeof(cmdLine), stdin);
    strcpy(cmdLine, lineRead);
}

int existCommand() {

    if (cmdElems[0] == NULL) {
        return 0;
    }

    int j = 0;
    for (j = 0; j < MAX - 1; j++) {
        if (strings[j] == '\0')
            break; // à ajouter pour éviter d'atteindre a fin de la chaine
        //printf("%s      %s\n",strings[j],cmdElems[0]);
        if (strcmp(cmdElems[0], strings[j]) == 0) {
            return 1;
        }
    }
}

int isExitCommand() {
    if (strcmp(cmdElems[0], "exit") == 0) {
        return 1;
    }
    return 0;
}

int isPwdCommand() {
    if (strcmp(cmdElems[0], "pwd") == 0) {
        return 1;
    }
    return 0;
}

int isCdCommand() {
    if (strcmp(cmdElems[0], "cd") == 0) {
        return 1;
    }
    return 0;
}

void separateCommand() {

    char* bufferPointer = cmdLine;

    int i;
    for (i = 0; i <= MAX; i++) {
        while (*bufferPointer && isspace(*bufferPointer))
            bufferPointer++;

        if (!*bufferPointer)
            break;

        /* on se souvient du début de ce mot */
        cmdElems[i] = bufferPointer;

        /* cherche la fin du mot */
        while (*bufferPointer && !isspace(*bufferPointer))
            bufferPointer++; /* saute le mot */

        /* termine le mot par un \0 et passe au suivant */
        if (*bufferPointer) {
            *bufferPointer = 0;
            bufferPointer++;
        }
    }

    cmdElems[i] = NULL;
}

void wait_l(pid_t pid) {
    int status;
    while (waitpid(pid, &status, 0) < 0) {

        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) == 0) {
            }
            //printf("le fils s'est terminé normalement avec le code 0");
            else {
                continue;
                printf("le fils s'est terminé avec le code %i",
                        WEXITSTATUS(status));
            }
        } else
            printf("terminaison par le signal %i\n", WTERMSIG(status));
        break;
    }
}

void execCommand(char* result) {
    pid_t pid;
    int fp;
    char cmdOut[2048];
    ///
    int fd[2];
    pipe(fd);
    //char lu[10];

    if (!cmdElems[0])
        return;  // si le premier element est NULL on arrete
    if (cmdElems[0] == NULL)
        return;

    // if (existCommand()==1) //ne fonctionne pas il faut trouver pourquoi
    {
        pid = fork();
        if (pid == 0) {

            close(fd[0]);
            // on lie stdout à l'entrée du fils
            dup2(fd[1], fileno(stdout));
            //dup2(fd[1], fileno(stderr));
            close(fd[1]);
            int i = execvp(cmdElems[0], cmdElems);
            if (i < 0) {
                close(fd[1]);
                printf("error");
                exit(0);
            }
            //close(fd[1]);
            exit(0);
        } else {
            close(fd[1]);

            memset(&cmdOut[0], 0, sizeof(cmdOut));
            int n;
            while (n = read(fd[0], cmdOut, 2048) > 0) {
                if (strcmp(cmdOut, "error") == 0) {
                    return;
                }

                printf("%s", cmdOut);

                if (strcmp(cmdOut, result) == 0) {
                    printf("\n\nBravo! level ok.\n\n");
                    exit(0);
                }
            }

            waitpid(pid, 0, 0);
        }
    }
}

int changeDir(char *dir) {
    //faire les bons tests comme quoi on ne sort pas du repertoire de travail
    return chdir(dir);
}

void handlePwdCommand() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        printf("Error while getting cwd");
    }
}

char* getRealPath(char* lastDir, char* curDir) {
    pid_t pid;
    char cmdOut[2048];
    ///
    int fd[2];
    pipe(fd);
    pid = fork();
    if (pid == 0) {

        close(fd[0]);
        // on lie stdout à l'entrée du fils
        dup2(fd[1], fileno(stdout));

        close(fd[1]);

        char newDir[2048];

        strcpy(newDir, lastDir);
        strcat(newDir, "/");
        strcat(newDir, curDir);

        changeDir(newDir);

        char newChangedDir[2048];
        getcwd(newChangedDir, sizeof(newChangedDir));
        printf("%s", newChangedDir);

        exit(0);
    } else {
        close(fd[1]);

        memset(&cmdOut[0], 0, sizeof(cmdOut));
        read(fd[0], cmdOut, 2048);

        //printf("%s", cmdOut);

        waitpid(pid, 0, 0);
        return cmdOut;
    }

}

int isValidDirectory(char* dir){
    struct stat fileStat;
        if(stat(dir,&fileStat) >= 0){
            if(S_ISDIR(fileStat.st_mode) && (fileStat.st_mode & S_IWUSR)&&(fileStat.st_mode & S_IXUSR)){
                return 1;
            }
        }
    return 0;
}

int startsWith(const char *first, const char *second) {
    size_t firstSize, secondSize;

    firstSize = strlen(first);
    secondSize = strlen(second);

    if (secondSize <= firstSize && strncmp(first, second, secondSize) == 0) {
        return 1;
    }
    return 0;
}

void handleCdCommand() {
    char lastDir[1024];
    getcwd(lastDir, sizeof(lastDir));

    if (cmdElems[1] == NULL) {
        changeDir(rwd);
    } else {

        char *currDir;

        /* get the first token */
        currDir = strtok(cmdElems[1], "/");

        char* lastDirPtr = &lastDir[0];

        /* walk through other tokens */
        while (currDir != NULL) {
            //printf(" %s\n", currDir);

            lastDirPtr = getRealPath(lastDirPtr, currDir);

            if(startsWith(lastDirPtr,rwd) != 1){ // on a quitté la racin du jeu.
                printf("You can not leave the root directory!\n");
                return;
            }

            if(isValidDirectory(lastDirPtr) !=1){ // on a vérifie le dossier
                printf("No access rights on folder \n : %s\n",lastDir);
                return;
            }

            currDir = strtok(NULL, "/");
        }
        changeDir(lastDirPtr);
    }

}

int main(int argc, char *argv[]) {

    int i = 0;
    char* args[7];
    args[0] = "tar";
    args[1] = "xzvf";
    args[2] = argv[1];
    args[3] = "-C";
    args[4] = "./lol";
    args[5] = NULL;

    FILE *fp;
    char *line = NULL;
    char *result = NULL;
    size_t len = 10;
    ssize_t read;

    if (!(folderExists("lol"))) {

        mode_t mask = umask(0); //on met le umask à 0 (pas de restrictions)
        result_code = mkdir("lol", 0700); //on crée le répertoire
        umask(mask); //on remet les droits initiaux
    } else {
        result_code = 0;
    }
    switch (result_code) {

    case -1:

        perror(argv[0]);
        printf(
                "Erreur dans la création du répertoire à noter que le réprtoire n'existe pas\n");
        exit(EXIT_FAILURE);
        break;

    case 0:

        printf("Le répertoire a bien été crée \n");

        pid_t pid;
        int status;

        if ((pid = fork()) < 0) {
            printf("Erreur dans le fork\n");
            exit(1);
        } else if (pid == 0) {
            //chdir("/home/kejji/proj/lol");
            if (execvp(*args, args) < 0) {
                printf("Erreur dans exec\n");
                exit(1);
            }

        } else {
            while (wait(&status) != pid)
                ;
        }

        if (changeDir("./lol/") != 0) {
            printf("Erreur d'acces au repertoire\n");
            exit(EXIT_FAILURE);
            break;
        }

        getcwd(rwd, sizeof(rwd));

        fp = fopen("./meta", "r"); //on ouvre le fichier à lire dans fp
        if (fp == NULL) {
            fprintf(stderr, "Erreur dans l'ouverture du fichier\n");
            exit(EXIT_FAILURE);
        }

        while ((read = getline(&line, &len, fp)) != -1) //getline() fait un malloc automatique pour line
        {

            if (strchr(line, '#') != NULL) {

            }

            else if (strchr(line, '$') != NULL) {
                strings[i] = strdup(line + 2);
                strings[i] = strtok(strings[i], "\n");
                i++;

            }

            else if (strchr(line, '>') != NULL) {
                result = strdup(line + 2);
                //result = strtok(result, "\n");

            }

        }
        strings[i] = "cd";

        unlink("./meta"); // il reste à gérer correctement le répertoire de travail!!!
        fclose(fp);
        free(line);
        //free(result);

        break;

    default:

        break;

    }

    while (1) {
        showPrompt();
        getCommand();
        separateCommand(); //(ligne, elems, MAXELEMS);

        if (isPwdCommand() == 1) {
            handlePwdCommand();
            continue;
        }

        if (isCdCommand() == 1) {
            handleCdCommand();
            continue;
        }

        if (isExitCommand() == 1) {
            exit(0);
        }

        if (existCommand() == 1) // si cette commande est valide, l'exécuter sinn inviter le joueur à reintroduire une autre commande
                {
            //printf("lol_99\n");
            execCommand(result);
        } else {
            printf("Les seules commandes autorisées sont :\n");
            for (i = 0; strings[i] != '\0'; i++) {
                printf("%d : %s   \n", i, strings[i]);
            }
        }

    }
    return 0;
}
