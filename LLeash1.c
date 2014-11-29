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


#define MAX 20

char cmdLine[1000];
char *cmdElems[MAX];// pointe sur les mots d'une ligne de commande

void showPrompt()
{
    printf("[LEASH]");
    fflush(stdout); // vider le buffer
}

void getCommand()
{
    fgets(cmdLine,sizeof(cmdLine),stdin);

}

void separateCommand()
{
    char* bufferPointer = cmdLine;

    // bufferPointer = strtok(cmdLine, " ");
    int i;
    for (i=0; i<=MAX; i++)
    {
        while (*bufferPointer && isspace(*bufferPointer)) bufferPointer++;

        if (!*bufferPointer) break;

        /* on se souvient du début de ce mot */
        cmdElems[i] = bufferPointer;

        /* cherche la fin du mot */
        while (*bufferPointer && !isspace(*bufferPointer)) bufferPointer++; /* saute le mot */

        /* termine le mot par un \0 et passe au suivant */
        if (*bufferPointer)
        {
            *bufferPointer = 0;
            bufferPointer++;
        }
    }

    cmdElems[i] = NULL;
}

void wait_l(pid_t pid)
{
    int status;
    while (waitpid(pid,&status,0)<0)
    {

        if (WIFEXITED(status))
        {
            if(WEXITSTATUS(status)==0)
                printf("le fils s'est terminé normalement avec le code 0");
            else
            {
                continue;
                printf("le fils s'est terminé avec le code %i",WEXITSTATUS(status));
            }
        }
        else
            printf("terminaison par le signal %i\n",WTERMSIG(status));
        break;
    }

}


void execCommand()
{
    pid_t pid;
    int fp;
    int fd;


    if (!cmdElems[0]) return;  // si le premier element est NULL on arrete

    if (fork() < 0)
    {
        perror("le processus fils n'a pas été crée");
    }

    if (fork()==0)
    {

        int i;



        //
        execvp(cmdElems[0],cmdElems);
    }
    else
    {// attendre que fils ait fini
        wait_l(pid);
    }
}

int main(int argc, char *argv[])
{


char *args[3] = {"tar xzvf argv", "-C /home/kejji/proj/", NULL};
FILE *fp;
char *line = NULL;
size_t len = 0;
ssize_t read;
char *strings[MAX]; //tableau dans lequel on va enregistrer les commandes possibles pour l'utilisateur
int i=0;

mode_t mask = umask(0); //on met le umask à 0 (pas de restrictions)
int result_code = mkdir("/home/kejji/proj/", 0777); //on crée le répertoire
umask(mask); //on remet les droits initiaux

  if(result_code ==-1) {
	  perror(argv[0]);
	  printf("Erreur dans la création du répertoire/n");
	  exit(EXIT_FAILURE);
  }

  if(result_code==0){

    printf("Le répertoire a bien été crée");
    execvp(args[0],args); //on exécute le programme tar pour décompresser le fichier
    fp = fopen("/home/kejji/proj/", "r"); //on ouvre le fichier à lire dans fp
    if (fp == NULL) {
              fprintf(stderr, "Erreur dans l'ouverture du fichier\n");
              exit(EXIT_FAILURE);
    }


    while ((read = getline(&line, &len, fp)) != -1) {
               printf("On récupère une ligne de taille %zu :\n", read);
               printf("%s", line);
               strings[i]=strdup(line);
               i++;

           }
    unlink("meta");

      free(strings[i]); //incomplet (pour ne pas oublier)
      free(line);
      fclose(fp);
      exit(EXIT_SUCCESS);
      }
    while (1)
    {
        showPrompt();
        getCommand();
        separateCommand();
        execCommand();
    }
    return 0;
}

