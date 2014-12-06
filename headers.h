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



//permet au TAB de rechercher si un début de commande correspond à une commande de ce tableau
static char *commandArray[] = {"login", "shutdown", "halt", "reboot", "install", "mount",
                            "umount", "chsh", "exit", "last", "file", "mkdir", "grep",
                           "dd", "find", "mv", "ls", "dif", "cat", "ln", "df", "top",
                           "free", "quota", "at", "lp", "adduser", "groupadd", "kill",
                           "kill", "crontab", "ifconfig", "ip", "ping", "netstat",
                           "telnet", "ftp", "route", "rlogin", "rcp", "finger", "mail",
                           "nslookup", "passwd", "su", "umask", "chgrp", "chmod","chown",
                           "chattr", "sudo", "ps", "tar", "unzip", "gunzip", "unarj",
                           "mtools", "man", "unendcode", "uudecode", "which", "who", "echo"};


int list_index;  //position actuelle dans le mot à compléter
int len;         //longueur du mot à compléter
const char *name;//nom de la commande qui correspond à ce que l'utilisateur rentre
char **matches = NULL; //on initialise les correspondances à NULL
char* bufferPointer;  //pointeur sur une commande qui va recevoir strtok()
size_t size;

#define MAX 10 //taille du tableau des commandes autorisées par meta
#define BUFFER_MAX_LENGTH 50 //taille maximale du buffer(tableau qui contient les commandes de l'utilisateur)
#define TRUE 1
static char buffer[BUFFER_MAX_LENGTH];  //définition du buffer
static int buffin = 0;

static char *commandArgv[5];  //tableau qui va contenir les commandes rentrées par l'utilisateur
static int commandArgc = 0;

char *argsuperior[5]; //tableau dans lequel on entre la partie droite du ">"
char *arginferior[5]; //tableau dans lequel on entre la partie droite du "<"

int fd[2]; //file descriptors utilisés pour gérer les pipes

char *argvA[5]; //tableau qui va contenir la commande A dans le pipe A|B
char *argvB[5]; //tableau qui va contenir la commande B du pipe

char* args[6]; //tableau qui va contenir les paramètres de la fonction tar


FILE *fp;
FILE *file;
FILE * file2;
char mystring;
char *line = NULL;
char *result = NULL;
size_t len = 0;
ssize_t read;
char *strings[MAX]; //tableau dans lequel on va enregistrer les commandes possibles pour l'utilisateur

/*********************************************************************
 Fonctions qui permettent de réaliser le TAB
 *********************************************************************/

void initialize_readline ();
char **command_completion (char *text, int start, int end); //permet de compléter la commande et de la retourner
char *command_generator (char *text, int state);


/*********************************************************************
 Fonctions qui permettent d'exécuter une commande
 *********************************************************************/

void cleanCommand();
void getCommand();
void separateCommand();


/*********************************************************************
 Fonctions qui permettent de gérer les signaux
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
