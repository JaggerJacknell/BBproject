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

/*********************************************************************
 Fonctions qui permettent de gérer les commandes
 *********************************************************************/

void cleanCommand()  //cleanCommand permet de nettoyer le tableau des commandes
{                     //commandArgc est le nombre de commandes de l'utilisateur
                      //commandargv[] est le tableau qui contient les commandes de l'utilisateur
  while (commandArgc != 0) {    //tant que l'on a des commandes dans le tableau

         commandArgv[commandArgc] = NULL;  //on supprimme le pointeur sur la commande
         commandArgc--;                    // on enlève la commande du tableau
      }

  //buffin = 0;                //on nettoie le buffer qui contient les chars rentrés par l'utilisateur

}


void separateCommand()
{
        
    char* bufferPointer = cmdLine;                                  // on déclare un pointeur sur le buffer
    int i;
    for (i = 0; i <= MAX; i++) {
			
			while (*bufferPointer && isspace(*bufferPointer))
			   bufferPointer++;
			
			if (!*bufferPointer)
			   break;
			   
			/* on se souvient du début de ce mot */
			commandArgv[i] = bufferPointer;
			/* cherche la fin du mot */
			while (*bufferPointer && !isspace(*bufferPointer))
				bufferPointer++; /* saute le mot */
				
			/* termine le mot par un \0 et passe au suivant */
			if (*bufferPointer) {

			   *bufferPointer = 0;
			   bufferPointer++;
		}
	}

	commandArgv[i] = NULL;
	
}
       /* bufferPointer = strtok(buffer, " ");                            //on scinde le buffer en commandes séparées par des espaces ex :{cat, README, NULL} et on pointe sur la 1ere commande (ici cat)
        while (bufferPointer != NULL) {                                 // tant que le pointeur n'est pas NULL (fin du buffer)
                commandArgv[commandArgc] = bufferPointer;               // on passe les commandes du buffer dans le tableau des commandes
                bufferPointer = strtok(NULL, " ");                      //on passe à la 2nde commande (ici README). Lors du second appel de strtok sur la meme chaine, le paramètre est NULL(convention)
                commandArgc++;                                          //ainsi de suite jusqu'à tomber sur NULL (fin du buffer), on stocke dans le tableau...
        }
        commandArgv[commandArgc++] = NULL;*/


//permet tout simplement d'exécuter les commandes rentrées par l'utilisateur
void execCommand(char* result)
{ 
  
  char cmdOut[2048];
  int fdd[2];
  pipe(fdd);
  
  /* Avant de lancer la commande dans le fork(), on va appeler sigaction.
  On va donc désactiver le CTRL-C.*/
  signal(SIGINT, SIG_IGN); //on ignore le CTRL-C...
  
  if (!commandArgv[0])
		return;
  if (commandArgv[0] == NULL)
	   return;
	   
  pid_t pid = fork();
  if (pid<0) {
    perror("Le processus fils n'a pas été crée");
  }
  if (pid==0) {                            //lorsqu'on se trouve dans le fils...
    signal(SIGINT, SIG_DFL);                  //on réactive le CTRL-C
    signal(SIGINT, handle_signal);            //on l'envoie au handler
    
    close(fdd[0]);
    dup2(fdd[1],fileno(stdout));
    close(fdd[1]);
                      
    int i = execvp(commandArgv[0],commandArgv); //...alors on l'exécute
  

      if(i<0){                                //sinon, si la commande n'existe pas
		    
        close(fdd[1]);  
        printf("%s : %s\n", commandArgv[0], "La commande n'existe pas ou n'est pas permise");
        exit(0);
      }

      exit(0);
    

} else {
	 close(fdd[1]);
	 memset(&cmdOut[0], 0, sizeof(cmdOut));
     int n;
   
     while (n = read(fdd[0], cmdOut, 2048) > 0) {

	     if (strcmp(cmdOut, "La commande n'existe pas ou n'est pas permise") == 0) {
            return;
          }

    printf("%s", cmdOut);
    
    if(strcmp(cmdOut, result) == 0) {
		printf("\n\nBravo! level ok.\n\n");
		exit(0);
	}
    signal(SIGINT, SIG_DFL); //on réactive le CTRL-C
    signal(SIGINT, handle_signal);//on l'envoie au handler
}

	waitpid(pid,0,0);

	}
}

//check si la fonction existe en comparant les commandes entrées par l'utilisateur avec le tableau des commandes du fichier meta
int existCommand()
{
	if (commandArgv[0] == NULL) {
		return 0;
	}
	
  int j;
  for(j=0;j<MAX-1;j++) {
	  if(strings[j] == '\0')
	  break; //A ajouter pour éviter d'atteindre la fin de la chaîne
    if (strcmp(commandArgv[0],strings[j])==0) {
      return 1;
    }
  }
  //return 0;
}

int changeDir(char* dir) {
  //faire les bons tests comme quoi on ne sort pas du repertoire de travail
  return chdir(dir);
}


int folderExists(char *ptrFile) // pour éviter de créer le répertoire deux fois...
{
// Renvoie 1 si existe, 0 sinon
    struct stat s;
    return (stat(ptrFile, &s) == 0);
}


void handlePwdCommand() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        printf("Erreur d'acquisition du chemin courant");
    }
}


void handleCdCommand() {
    char lastDir[1024];
    getcwd(lastDir, sizeof(lastDir));

    if (commandArgv[1] == NULL) {
        changeDir(rwd);
    } else {

        char *currDir;

       
        currDir = strtok(commandArgv[1], "/");

        char* lastDirPtr = &lastDir[0];

       
        while (currDir != NULL) {
            

            lastDirPtr = getRealPath(lastDirPtr, currDir);

            if(startsWith(lastDirPtr,rwd) != 1){ // on a quitté la racin du jeu.
                printf("Vous ne pouvez pas quitter le répertoire racine!\n");
                return;
            }

            if(isValidDirectory(lastDirPtr) !=1){ // on a vérifie le dossier
                printf("Pas de doits d'accès sur le fichier \n : %s\n",lastDir);
                return;
            }

            currDir = strtok(NULL, "/");
        }
        changeDir(lastDirPtr);
    }

}

char* getRealPath(char* lastDir, char* curDir) {
    pid_t pid;
    char cmdOut[2048];
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
/*********************************************************************
 Fonctions qui permettent de gérer les signaux
 *********************************************************************/

void wait_l(pid_t pid)  //Gère les différents signaux
{
  int status;
  while (waitpid(pid,&status,0)<0)
    {

        if (WIFEXITED(status))
        {
            if(WEXITSTATUS(status)==0)
                printf("Le fils s'est terminé normalement avec le code 0 \n");
            else
            {
                continue;
                printf("Le fils s'est terminé avec le code %i",WEXITSTATUS(status));
            }
        }
        else
            printf("Terminaison par le signal %i\n",WTERMSIG(status));
        break;
    }
}

void handle_signal(int signo) //handler qui permet la gestion du CTRL-C
{
  printf("\n[$LEASH] "); //On affiche le prompt
  fflush(stdout); //fflush s'assure que le Shell affiche bien le prompt directement même si le buffer n'est pas plein
}


/*********************************************************************
 Fonctions qui permettent de réaliser le TAB
 *********************************************************************/

/* text est la commande à compléter
   start est la borne inf du buffer (rl_line_buffer) qui contient le mot à compléter
   end est la borne sup du buffer qui contient le mot à compléter
   La fonction renvoie le tableau des correspondances avec le mot à compléter, ou NULL si il n'y a aucune correspondance*/

char **command_completion (const char *text, int start, int end) { //permet de compléter la commande et de la retourner

  char **matches;
  matches = (char **)NULL; //on initialise les correspondances à NULL


  /* Si le mot à compléter est en début de ligne, alors c'est une commande.
     Autrement, c'est le nom d'un fichier dans le répertoire courant. */

  if (start == 0)
    matches = rl_completion_matches ((char*)text, &command_generator);   //on complète la commande
  else
	rl_bind_key('\t',rl_abort);
	
  return matches; //puis on la retourne
}

/* state permet de savoir si l'on doit commencer la recherche du mot depuis le début
(si state == 0, alors on doit commencer la recherche depuis le début du mot)*/

char *command_generator (const char *text, int state) {

  static int list_index;  //position actuelle dans le mot à compléter
  static int len;         //longueur du mot à compléter
  char *name;//nom de la commande qui correspond à ce que l'utilisateur rentre


  if (!state)                             //si state = 0 (c'est un nouveau mot)...
    {
      list_index = 0;                     //...alors on initialise l'index à 0 (on part du début du tableau)
      len = strlen (text);                //on enregistre la longueur du mot dans la variable len
    }


  while ((name == commandArray[list_index]))  //tant que la commande entrée par l'utilisateur correspond à une commande du tableau des commandes autorisées
    {
      list_index++;                        //on incrémente l'index

      if (strncmp (name, text, len) == 0) //lorsque la commande entrée par l'utilisateur correspond entièrement à la commande autorisée...
        return (strdup(name));             //...on retourne une copie
    }


  return ((char *)NULL);                   //s'il n'y a pas de correspondances avec une commande autorisée alors on retourne NULL
}



/*********************************************************************
 Fonctions qui permettent de réaliser les redirections et pipes
 *********************************************************************/

//voici la fonction qui gère les redirections ">"
int SuperiorCommand(int i) { //le paramètre i représente la position du > dans  le tableau des commandes passées par l'utilisateur

  char *argsuperior[5];

  int j;
  for(j=0; j<i; j++) { //on parcours le tableau des commandes de l'utilisateur jusqu'à ">"
    argsuperior[j] = commandArgv[j]; //on stocke la partie à droite du > dans le tableau argv
    j++;
  }

  argsuperior[j] = NULL; //on met NULL à la fin du tableau


  return execSuperiorCommand(argsuperior, commandArgv[i+1]);
}

int execSuperiorCommand(char * argv[], char * filename) { //fonction qui permet d'effectuer la redirection ">"

    int pop = dup(1); //on crée un tube sur la sortie standard

    //On ouvre le fichier dans lequel on veut faire la redirection
    int file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRGRP | S_IROTH);

    if(file <0) { //si le fichier ne s'ouvre pas...
            printf("Le fichier de redirection ne peut pas s'ouvrir ou n'existe pas");
            return 1; } //...on affiche un message d'erreur

    //Autrement, on redirige la sortie standard vers le fichier de redirection
    else{

         if(dup2(file,1)< 0){ //si la redirection plante...
                printf("La sortie standard n'a pu être redirigée vers le fichier de redirection");
                return 1; }//...on affiche un message d'erreur

                else{ //si la redirection fonctionne...

                      int pid = fork();
                      if( pid == 0) {
                        close(file); //on ferme les descripteurs utilisés pour pouvoir exécuter la commande
                        close(pop);
                        execvp(argv[0], argv); //et on exécute la commande dans le fichier de redirection
                    return 0;
                   }
                }
             }

    dup2(pop, 1); //on redirige la sortie standard vers le descripteur fd
    close(file);  //on ferme les descripteurs (on a fini de les utiliser)
    close(pop);
    wait(NULL);  //on attend que le fils se termine
    close(file);
  return 0;
}

//voici la fonction qui gère les redirections "<"
int InferiorCommand(int i) { //le paramètre i représente la position du < dans  le tableau des commandes passées par l'utilisateur

  char *arginferior[5];

  int j;
  for(j=0; j<i; j++) { //on parcourt le tableau de commande jusqu'à tomber sur "<"
    arginferior[j] = commandArgv[j]; //on stocke la partie à droite du "<" dans le tableau argv
    j++;
  }

  arginferior[j] = NULL; //on met NULL à la fin du tableau


  int pid, status;
  fflush(stdout);

    switch ((pid = fork())) {
    case -1:
        perror("fork");
        break;
    case 0:

        executeInferiorCommand(arginferior, commandArgv[i+1]);
        break;
    default:
        pid = wait(&status);
        return 0;
    }

  return 0;
}


int executeInferiorCommand(char * argv[], char * filename) {

  int fd[2];
  if(pipe(fd)) {
    perror("pipe");
    exit(1);
  }

  switch(fork()) {
  case -1:
    perror("fork()");
    exit(1);
  case 0:
        close(fd[0]);  
        dup2(fd[1], 1);  
        close(fd[1]);  

      FILE * file2;
      char mystring;

      file2 = fopen (filename , "r");

      if (file2 == NULL) {
        printf ("Le fichier de redirection ne peut pas s'ouvrir ou n'existe pas");
      }
      else {

     while ((mystring=fgetc(file2)) != EOF) {
         putchar(mystring); 
       }
        fclose (file2);
      }
     exit(0);

    default:

        close(fd[1]);  
        dup2(fd[0], 0);  
        close(fd[0]);  

        execvp(argv[0], argv);
        perror(argv[0]);
        exit(1);


  }

  return 0;
}


int PipedCommand(int i) {
  char *argvA[5];
  char *argvB[5];


  int ii;
  for(ii=0;ii<i;ii++) { //on parcourt le tableau de commande jusqu'à tomber sur "|"
    argvA[ii] = commandArgv[ii]; //on stocke la partie à droite du "|" dans le tableau argvA (1ere commande)
    ii++;
}

  argvA[ii]=NULL; //on met NULL à la fin du tableau argvA


  int j,jj=0;
  for(j=i+1; j<commandArgc; j++) { //on parcourt le tableau de commande jusqu'à la fin du tableau
    argvB[jj] = commandArgv[j]; //on stocke la partie à gauche du "|" dans le tableau argvB (2nde commande)
    jj++;
  }

  argvB[jj]=NULL;


  int pid, status;
  fflush(stdout);

    switch ((pid = fork())) {
    case -1:
        perror("fork");
        break;

    case 0:

      executePipedCommand(argvA, argvB);
        break;

    default:
        pid = wait(&status);
        return 0;
    }
    return 1;
}


void executePipedCommand(char *argvA[], char  *argvB[]) {

  int fd[2];

  if(pipe(fd)){
    perror("pipe");
    exit(1);
  }

  else{

    switch(fork()) {
      case -1:
        perror("fork()");
        exit(1);

      case 0: //on exécute le procssus A dans le fils
        close(fd[0]);
        dup2(fd[1], 1);
        close(fd[1]);

        execvp(argvA[0], argvA);
        perror(argvA[0]);
        exit(1);

      default: //on doit exécuter la dernière commande du pipe dans le père car c'est celui-çi qui devra être écouté par le SHELL (et attendre la fin du processus A)
        close(fd[1]);
        dup2(fd[0], 0);
        close(fd[0]);

        execvp(argvB[0], argvB);
        perror(argvB[0]);
        exit(1);


  }
 }
}

/*********************************************************************
 Fonctions qui permettent de réaliser les travaux en foreground et background
 *********************************************************************/


void executeCommand(char *command[], int newDescriptor, int executionMode)
{
        int commandDescriptor;
        /**
         *  Définit les entrées/sorties du nouveau processus
         */
        if (newDescriptor == STDIN) {
                                                      
                dup2(commandDescriptor, STDIN_FILENO);
                close(commandDescriptor);
        }
        if (newDescriptor == STDOUT) {
                
                dup2(commandDescriptor, STDOUT_FILENO);
                close(commandDescriptor);
        }
        if (execvp(command[0], command) == -1)
                perror("LEASH-shell(execvp)");
}


void launchJob(char *command[], int newDescriptor, int executionMode)
{
        pid_t pid;
        pid = fork();
        switch (pid) {
        case -1:
                perror("LEASH-shell(fork)");
                exit(EXIT_FAILURE);
                break;
        case 0:
                /**
                 *  on réactive les signaux que l'on avait désactivé pour l'initialisation
                 */
                signal(SIGINT, SIG_DFL);
                signal(SIGQUIT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
                signal(SIGCHLD, &signalHandler_child);
                signal(SIGTTIN, SIG_DFL);

                usleep(20000);                                                                                                                                           
                tcsetpgrp(LEASH_TERMINAL, getpid());    //met notre processus en foreground sur le terminal LEASH

                if (executionMode == BACKGROUND)
                        printf("[%d] %d\n", ++numActiveJobs, (int) getpid());   // on informe l'utilisateur qu'un nouveau processus est placé en background
                
                executeCommand(command, newDescriptor, executionMode);

                exit(0);
                break;

        default:
                setpgid(pid, pid);                                                     // on place notre processus en leader dans le groupe de processus
                jobsList = insertJob(pid, pid, *(command),(int) executionMode);       // on insère le job dans la liste

                t_job* job = getJob(pid, BY_PROCESS_ID);                             

                if (executionMode == FOREGROUND) {
                        putJobForeground(job, FALSE);                                // met le job en foreground
                }
                if (executionMode == BACKGROUND)
                        putJobBackground(job, FALSE);                                // met le job en background
                break;
        }
}

/**
 * Cette fonction permet de mettre un job en foreground. Si continueJob = TRUE, elle envoie un signal SIGCONT pour lui dire
 réveille toi tu peux continuer. Une fois qu'il a terminé, on rend la main au shell.
 */
void putJobForeground(t_job* job, int continueJob)
{
        job->status = FOREGROUND;                                         // on met le statut du job à FOREGROUND
        tcsetpgrp(LEASH_TERMINAL, job->pgid);                              // on donne au job le contrôle du terminal
        if (continueJob) {                                                // si on veut continuer le job...
                if (kill(-job->pgid, SIGCONT) < 0)                        // ...on lui envoie un SIGCONT
                        perror("kill (SIGCONT)");
        }

        waitJob(job);                                                     // on attend la fin du job...
        tcsetpgrp(LEASH_TERMINAL, LEASH_PGID);                              // une fois terminé, on rend le contrôle du shell au terminal
}

/**
 * Cette fonction permet de mettre un job en background. Si continueJob = TRUE, elle envoie un signal SIGCONT pour lui dire
 réveille toi tu peux continuer. Une fois qu'il a terminé, on rend la main au shell.
 */
void putJobBackground(t_job* job, int continueJob)
{
        if (job == NULL)
                return;

        if (continueJob && job->status != WAITING_INPUT) //si on veut continuer le processus et qu'il n'est pas en état d'attente...
                job->status = WAITING_INPUT;  //on le met en état d'attente 

        if (continueJob)   //si on veut seulement continuer le processus...                     
                if (kill(-job->pgid, SIGCONT) < 0) //...alors on envoie un SIGCONT
                        perror("kill (SIGCONT)");

        tcsetpgrp(LEASH_TERMINAL, LEASH_PGID);      //On rend la main au terminal                       
}

/**
 * on attend la fin des processus. Une fois qu'il sont exécutés ou suspendus, on les supprime de la liste
 *
 */
void waitJob(t_job* job)
{
        int terminationStatus;
        while (waitpid(job->pid, &terminationStatus, WNOHANG) == 0) {      // tant qu'il y a des processus à attendre...
                if (job->status == SUSPENDED)                              // si le job est suspendu...
                        return;                                            //...alors on arrête
        }
        jobsList = delJob(job);                                            // on supprime le job
}

/**
 * permet de tuer un job selon son pid
 */
void killJob(int jobId)
{
        t_job *job = getJob(jobId, BY_JOB_ID);                             // on prend le job correspondant au pid indiqué
        kill(job->pid, SIGKILL);                                          // on lui envoie un SIGKILL (et bimm!)
}

/**
*permet d'insérer un job dans la liste chaînée en renseignant son pid, son pgid, son nom, statut et descripteur
*/
t_job* insertJob(pid_t pid, pid_t pgid, char* descriptor, int status)
{
        usleep(10000);

        t_job *newJob = malloc(sizeof(t_job)); //on alloue de la mémoire pour le nouveau job
        newJob->pid = pid;
        newJob->pgid = pgid;
        newJob->status = status;
        newJob->descriptor = (char*) malloc(sizeof(descriptor));
        newJob->descriptor = strcpy(newJob->descriptor, descriptor);
        newJob->next = NULL;

        if (jobsList == NULL) { //si la liste est vide...
                numActiveJobs++; //alors on incrémente la liste d'un job
                newJob->id = numActiveJobs;//le nombre de jobs actifs correspond à l'id du job
                return newJob;//on retourne le job que l'on vient de créer

        } else { //si la liste de départ n'est pas vide
                t_job *auxNode = jobsList;
                while (auxNode->next != NULL) { //on détermine la place à laquelle on va pouvoir insérer le nouveau job
                        auxNode = auxNode->next;
                }
                newJob->id = auxNode->id + 1;
                auxNode->next = newJob;//on insère le nouveau job
                numActiveJobs++; //on incrémente le nombre de jobs présents dans la liste
                return jobsList;
        }
}

/**
 * Cette fonction permet de modifier le statut d'un processus en fonction de son pid
 */
int changeJobStatus(int pid, int status)
{
        usleep(10000);
        t_job *job = jobsList;
        if (job == NULL) {   //si la liste est vide...
                return 0;    //...on renvoie 0
        } else {
                int counter = 0;
                while (job != NULL) {
                        if (job->pid == pid) { //on parcours la liste en fonction du pid du processus
                                job->status = status; //on renvoie le statut du processus
                                return TRUE;
                        }
                        counter++;
                        job = job->next;
                }
                return FALSE;
        }
}

/**
 *  Permet de supprimmer un processus qui n'est plus actif de la liste chaînée
 */
t_job* delJob(t_job* job)
{
        usleep(10000);
        if (jobsList == NULL) //si la liste est vide...
                return NULL;  //...on renvoie NULL

        t_job* currentJob; //pointeur vers le processus en cours
        t_job* beforeCurrentJob; //pointeur vers le processus suivant 

        currentJob = jobsList->next; //on fait pointer le pointeur vers le prochain processus
        beforeCurrentJob = jobsList; 

        if (beforeCurrentJob->pid == job->pid) { //si le pid du processus actuel correspond à celui du processus précédent dans la liste

                beforeCurrentJob = beforeCurrentJob->next; //alors on passe au suivant (le processus est accomplit)
                numActiveJobs--; //on décrémente le nombre de processus actifs
                return currentJob; //on retourne le processus actuel
        }

        while (currentJob != NULL) { //tant qu'il reste des processus à réaliser
                if (currentJob->pid == job->pid) {
                        numActiveJobs--;
                        beforeCurrentJob->next = currentJob->next;
                }
                beforeCurrentJob = currentJob;
                currentJob = currentJob->next;
        }
        return jobsList;
}

/**
 * searches a job in the active jobs list, by pid, job id, job status
 */
t_job* getJob(int searchValue, int searchParameter)
{
        usleep(10000);
        t_job* job = jobsList;
        switch (searchParameter) {
        case BY_PROCESS_ID:
                while (job != NULL) {
                        if (job->pid == searchValue)
                                return job;
                        else
                                job = job->next;
                }
                break;
        case BY_JOB_ID:
                while (job != NULL) {
                        if (job->id == searchValue)
                                return job;
                        else
                                job = job->next;
                }
                break;
        case BY_JOB_STATUS:
                while (job != NULL) {
                        if (job->status == searchValue)
                                return job;
                        else
                                job = job->next;
                }
                break;
        default:
                return NULL;
                break;
        }
        return NULL;
}

/**
 * répond à la command jobs (affiche les processus courants)
 */
void printJobs()
{
        printf("\nActive jobs:\n");
        printf(
                "---------------------------------------------------------------------------\n");
        printf("| %s  | %s | %s | %s | %s |\n", "job n°.", "name", "pid", "descriptor", "status");
        printf(
                "---------------------------------------------------------------------------\n");

        t_job* job = jobsList;

        if (job == NULL) {
                printf("| %s %62s |\n", "Pas de jobs actuellement en cours", "");
        } else {
                while (job != NULL) {
                        printf("|  %d | %s | %d | %s | %c |\n", job->id, job->name, job->pid, job->descriptor, job->status);
                        job = job->next;
                }
        }
        printf(
                "---------------------------------------------------------------------------\n");

}
/* Un sous-shell interactif doit s'assurer qu'il a bien été placé en foreground par son shell parent avant de pouvoir gérer des jobs par lui même.
Pour cela, on se renseigne sur son PGID avec la fonction getpgrp puis on compare son PGID avec celui du processus actuellement en foreground.
Si les deux sont égaux, c'est qu'on a bien mis le shell en foreground, on peut donc maintenant gérer les jobs...
Les processus appartenant au même groupe que le terminal sont en foreground alors que les autres sont en background*/

void init_shell()
{
                                                                     
        LEASH_TERMINAL = STDIN_FILENO;                                       // terminal = STDIN
        LEASH_IS_INTERACTIVE = isatty(LEASH_TERMINAL);            // Le shell est interactif si STDIN est le terminal
        LEASH_PID = getpid();
        //on doit d'abord s'assurer que le Shell est interactif est en foreground
        if (LEASH_IS_INTERACTIVE) {                                                 // si le shell est interactif...
                while (tcgetpgrp(LEASH_TERMINAL) != (LEASH_PGID = getpgrp()))         //...on loop jusqu'à passer en foreground : pour cela on kill le shell en utilisant SIGTTIN jusqu'à ce que les deux PGID soient égaux
                        kill(LEASH_PID, SIGTTIN);                                                    

                /**
                 * on ignore les signaux d'arrêts  pour que notre shell ne soit pas stoppé accidentellement et on installe notre handler
                 */
                signal(SIGQUIT, SIG_IGN);
                signal(SIGTTOU, SIG_IGN);
                signal(SIGTTIN, SIG_IGN);
                signal(SIGTSTP, SIG_IGN);
                signal(SIGINT, SIG_IGN);
                signal(SIGCHLD, &signalHandler_child);



                setpgid(LEASH_PID, LEASH_PID);                      // on fixe le PID du shell à LEASH_PID
                LEASH_PGID = getpgrp();                              // on fixe le PGID du shell courant à LEASH_PGID
                if (LEASH_PID != LEASH_PGID) { //si les deux sont différents alors notre shell n'est pas le shell courant
                        printf("Erreur, le shell n'est pas leader");
                        exit(1);
                }
                if (tcsetpgrp(LEASH_TERMINAL, LEASH_PGID) == -1)      // si notre processus ne peut pas prendre le contrôle du LEASH_terminal (de STDIN)
                        tcgetattr(LEASH_TERMINAL, &LEASH_TMODES);             // on restaure les valeurs par défault pour le terminal

                
        } else {
                printf("On ne peut pas rendre LEASH interactif. Au revoir..\n");
                exit(1);
        }
}



/**
 * signal handler for SIGCHLD
 */
void signalHandler_child(int p)
{
        pid_t pid;
        int terminationStatus;

        pid = waitpid(WAIT_ANY, &terminationStatus, WUNTRACED | WNOHANG); //on attend la terminaison d'un processus
        if (pid > 0) {                                                                          
                t_job* job = getJob(pid, BY_PROCESS_ID);                  //on prend le job dans la liste
                if (job == NULL)
                        return;
                if (WIFEXITED(terminationStatus)) {                       //dans le cas où le processus s'est terminé normalement
                        if (job->status == BACKGROUND) {                             
                                printf("\n[%d]+  Processus terminé\t   %s\n", job->id, job->name); 
                                jobsList = delJob(job);                       //on le supprime de la liste
                        }
                } else if (WIFSIGNALED(terminationStatus)) {                 // dans le cas où le processus se termine à cause d'un signal
                        printf("\n[%d]+  Le processus a été tué\t   %s\n", job->id, job->name); 
                        jobsList = delJob(job);                            // on le supprime de la liste
                } else if (WIFSTOPPED(terminationStatus)) {                 // dans le cas où le processus se termine à cause d'un signal SIGSTP
                        if (job->status == BACKGROUND) {                           
                                tcsetpgrp(LEASH_TERMINAL, LEASH_PGID);
                                changeJobStatus(pid, WAITING_INPUT);                     // on change le statut du signal à "attente d'une entrée"
                                printf("\n[%d]+   Le processus a été suspendu [nécessite une entrée]\t   %s\n", numActiveJobs, job->name);                                  
                        } else {                                                                           // otherwise, the job is going to be suspended
                                tcsetpgrp(LEASH_TERMINAL, job->pgid);
                                changeJobStatus(pid, SUSPENDED);                         // we modify the status
                                printf("\n[%d]+   stopped\t   %s\n", numActiveJobs, job->name); // and inform the user
                        }
                        return;
                } else {
                        if (job->status == BACKGROUND) {                          // autrement, on supprime simplement le job
                                jobsList = delJob(job);
                        }
                }
                tcsetpgrp(LEASH_TERMINAL, LEASH_PGID);
        }
}
