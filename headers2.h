#include <stdio.h>
#include <stdlib.h>                                                                            // malloc()
#include <unistd.h>                                                                           // unix lib (fork, dup2 etc.)
#include <string.h>                                                                            // string functions
#include <sys/types.h>                                                                     // extra data types
#include <signal.h>                                                                            // signals
#include <sys/wait.h>                                                                        // wait functions
#include <fcntl.h>                                                                               // file descriptor operations
#include <termios.h>                                                                         // terminal control data structures
#include <sys/stat.h>  //mkdir
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/file.h>
#include <sys/errno.h>
#include <curses.h>
#include <term.h>

//permet au TAB de rechercher si un d�but de commande correspond � une commande de ce tableau
char *commandArray[] = {"login", "shutdown", "halt", "reboot", "install", "mount",
                            "umount", "chsh", "exit", "last", "file", "mkdir", "grep",
                           "dd", "find", "mv", "ls", "dif", "cat", "ln", "df", "top",
                           "free", "quota", "at", "lp", "adduser", "groupadd", "kill",
                           "kill", "crontab", "ifconfig", "ip", "ping", "netstat",
                           "telnet", "ftp", "route", "rlogin", "rcp", "finger", "mail",
                           "nslookup", "passwd", "su", "umask", "chgrp", "chmod","chown",
                           "chattr", "sudo", "ps", "tar", "unzip", "gunzip", "unarj",
                           "mtools", "man", "unendcode", "uudecode", "which", "who", "echo"};


int i;
int result_code;
int list_index;  //position actuelle dans le mot � compl�ter
int len;         //longueur du mot � compl�ter
const char *name;//nom de la commande qui correspond � ce que l'utilisateur rentre
char **matches = NULL; //on initialise les correspondances � NULL
char* bufferPointer;  //pointeur sur une commande qui va recevoir strtok()
size_t size;
size_t lenth = 0;
#define MAX 10 //taille du tableau des commandes autoris�es par meta
#define BUFFER_MAX_LENGTH 50 //taille maximale du buffer(tableau qui contient les commandes de l'utilisateur)
#define FOREGROUND 'F'
#define BACKGROUND 'B'
#define SUSPENDED 'S'
#define WAITING_INPUT 'W'
#define STDIN 1
#define STDOUT 2
#define BY_PROCESS_ID 1
#define BY_JOB_ID 2
#define BY_JOB_STATUS 3


static char *commandArgv[5];  //tableau qui va contenir les commandes rentr�es par l'utilisateur
static int commandArgc = 0;

char *argsuperior[5]; //tableau dans lequel on entre la partie droite du ">"
char *arginferior[5]; //tableau dans lequel on entre la partie droite du "<"

int fd[2]; //file descriptors utilis�s pour g�rer les pipes
int c;
char *argvA[5]; //tableau qui va contenir la commande A dans le pipe A|B
char *argvB[5]; //tableau qui va contenir la commande B du pipe

char* args[6]; //tableau qui va contenir les param�tres de la fonction tar

FILE *fp;
FILE *file;
FILE * file2;
char mystring;
char *line = NULL;
char *result = NULL;

char rwd[1024];
char* lineRead;
char cmdLine[1000];


ssize_t reado;
char *strings[MAX]; //tableau dans lequel on va enregistrer les commandes possibles pour l'utilisateur
pid_t pid;
int status;
static int numActiveJobs = 0; 							
typedef struct job { 									
        int id;
        char *name;
        pid_t pid;
        pid_t pgid; 
        int status;
        char *descriptor;
        struct job *next;
} t_job;

static t_job* jobsList = NULL; 	//liste chaînée des processus						


/* shell pid, pgid, default terminal, terminal modes */
static pid_t LEASH_PID;
static pid_t LEASH_PGID;
static int LEASH_TERMINAL, LEASH_IS_INTERACTIVE;
static struct termios LEASH_TMODES;

/*********************************************************************
 Fonctions qui permettent de r�aliser le TAB
 *********************************************************************/

void initialize_readline ();
static char **command_completion (const char *text, int start, int end); //permet de compl�ter la commande et de la retourner
char *command_generator (const char *text, int state);


/*********************************************************************
 Fonctions qui permettent d'ex�cuter une commande
 *********************************************************************/

void cleanCommand();
void getCommand();
void separateCommand();
void execCommand(char* result);
int existCommand();
int changeDir(char* dir);
int folderExists(char *ptrFile);
void handleCdCommand();
void handlePwdCommand();
int startsWith(const char *first, const char *second);
int isValidDirectory(char* dir);
char* getRealPath(char* lastDir, char* curDir);


/*********************************************************************
 Fonctions qui permettent de g�rer les signaux
 *********************************************************************/

void wait_l(pid_t pid);
void handle_signal(int signo);


/*********************************************************************
 Fonctions qui permettent de réaliser les redirections et pipes
 *********************************************************************/

int processCommand();
int SuperiorCommand(int i);
int execSuperiorCommand(char * argv[], char * filename);
int InferiorCommand(int i);
int executeInferiorCommand(char * argv[], char * filename);
int PipedCommand(int i);
void executePipedCommand(char *argvA[], char  *argvB[]);


/*********************************************************************
 Fonctions qui permettent de réaliser les foregrounds et backgrounds
 *********************************************************************/

 void handleUserCommand();
 void executeCommand(char *command[], int newDescriptor, int executionMode);
 void launchJob(char *command[], int newDescriptor, int executionMode);
 void putJobForeground(t_job* job, int continueJob);
 void putJobBackground(t_job* job, int continueJob);
 void waitJob(t_job* job);
 void killJob(int jobId);
 t_job* insertJob(pid_t pid, pid_t pgid, char* descriptor, int status);
 int changeJobStatus(int pid, int status);
 t_job* delJob(t_job* job);
 t_job* getJob(int searchValue, int searchParameter);
 void printJobs();
 void init_shell();
 void signalHandler_child(int p);
