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

#define MAX 200
char *strings[MAX]; //tableau dans lequel on va enregistrer les commandes possibles pour l'utilisateur


char cmdLine[1000];
char *cmdElems[MAX];// pointe sur les mots d'une ligne de commande
int result_code;

int folderExists(char *ptrFile) // ajout à tester pour éviter de créér le réprtoire lol deux fois!!!
{
// Renvoie 1 si existe, 0 sinon
    struct stat s;
    return (stat(ptrFile, &s) == 0);
}

void showPrompt()
{
    printf("[LEASH]»»");
    fflush(stdout); // vider le buffer
}

void getCommand()
{
    fgets(cmdLine,sizeof(cmdLine),stdin);

  }

int existCommand()
{

    int j=0;
    for(j=0; j<MAX-1; j++)
    {
        if (strings[j]=='\0') break; // à ajouter pour éviter d'atteindre a fin de la chaine
        //printf("%s      %s\n",strings[j],cmdElems[0]);
        if (strcmp(cmdElems[0],strings[j])==0)
        {
            return 1;
        }
    }
}


void separateCommand()
{

    char* bufferPointer = cmdLine;


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
            if(WEXITSTATUS(status)==0) {}
            //printf("le fils s'est terminé normalement avec le code 0");
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
    if (cmdElems[0]==NULL) return;
    if (fork() < 0)
    {
        perror("le processus fils n'a pas été crée");
    }

   // if (existCommand()==1) //ne fonctionne pas il faut trouver pourquoi
    {

        if (fork()==0)
        {

            int i = execvp(cmdElems[0],cmdElems);
            if(i<0)
            {
                printf("%s : %s\n", cmdElems[0], "La commande n'existe pas ou n'est pas permise");
                exit(1);
            }
        }
        else
        {
            // attendre que fils ait fini
            wait_l(pid);
        }

    }

}


int main(int argc, char *argv[])
{


    int i=0;
    char* args[7];
    args[0] = "tar";
    args[1] = "xzvf";
    args[2] = argv[1];
    args[3] = "-C";
    args[4] = argv[2];
    args[5] = NULL;

    FILE *fp;
    char *line = NULL;
    char *result = NULL;
    size_t len = 10;
    ssize_t read;

    if (!(folderExists("lol")))
    {

        mode_t mask = umask(0); //on met le umask à 0 (pas de restrictions)
        result_code = mkdir("lol",0700); //on crée le répertoire
        umask(mask); //on remet les droits initiaux
    }
    else
    {
        result_code=0;
    }
    switch(result_code)
    {

    case -1:

        perror(argv[0]);
        printf("Erreur dans la création du répertoire à noter que le réprtoire n'existe pas\n");
        exit(EXIT_FAILURE);
        break;


    case 0:


        printf("Le répertoire a bien été crée \n");

        pid_t  pid;
        int    status;

        if ((pid = fork()) < 0)
        {
            printf("Erreur dans le fork\n");
            exit(1);
        }
        else if (pid == 0)
        {
            //chdir("/home/kejji/proj/lol");
            if (execvp(*args, args) < 0)
            {
                printf("Erreur dans exec\n");
                exit(1);
            }
        }
        else
        {
            while (wait(&status) != pid)
                ;
        }

        fp = fopen("/home/kejji/proj/proji/lol/meta", "r"); //on ouvre le fichier à lire dans fp
        if (fp == NULL)
        {
            fprintf(stderr, "Erreur dans l'ouverture du fichier\n");
            exit(EXIT_FAILURE);
        }


        while ((read = getline(&line, &len, fp)) != -1)   //getline() fait un malloc automatique pour line
        {

            printf("On récupère une ligne de taille %zu :\n", read);
            printf("%s", line);
            if(strchr(line,'#') != NULL)
            {

            }

            else if (strchr(line,'$') != NULL)
            {
                strings[i] = strdup(line);

                //printf("%s",strings[i]);  strdup s'occupe déja d'écrire

                i++;

            }

            else if (strchr(line,'>') != NULL)
            {
                result = strdup(line);

            }

        }


        unlink("./lol/meta");// il reste à gérer correctement le répertoire de travail!!!
        fclose(fp);
        free(line);
        free(result);

        break;

    default:

        break;

    }

    while (1)
    {
        showPrompt();
        getCommand();
        separateCommand(); //(ligne, elems, MAXELEMS);

        if (existCommand()==1) // si cette commande est valide, l'exécuter sinon
            // inviter le joueur à reintroduire une autre commande
        {
            printf("lol_99\n");
            execCommand();
        }

    }
    return 0;
}

