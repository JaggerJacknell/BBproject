/*
  Correction basée sur sur le code d'Antoine Miné
*/
/* en-têtes standard */
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/wait.h>   /* pour avoir wait & co. */
#include <ctype.h>      /* pour avoir isspace & co. */
#include <string.h>
#include <errno.h>      /* pour avoir errno */
#include <signal.h>
#include <fcntl.h>

char   ligne[4096];     /* contient la ligne d'entrée */

/* pointeurs sur les mots de ligne (voir decoupe) */
#define MAXELEMS 32
char* elems[MAXELEMS];


void affiche_invite()
{
  printf("--> ");
  fflush(stdout);
}

void lit_ligne()
{
  if (!fgets(ligne,sizeof(ligne)-1,stdin)) {
    printf("\n");
    exit(0);
  }
}

/* découpe ligne en mots
   fait pointer chaque elems[i] sur un mot différent
   elems se termine par NULL
 */
void decoupe(char *ligne, char** elems, int maxelems) {
  int i;
  char* debut = ligne;
  for (i=0; i<maxelems-1; i++) {
    /* saute les espaces */
    while (*debut && isspace(*debut)) debut++;
    /* fin de ligne ? */
    if (!*debut) break;
    /* on se souvient du début de ce mot */
    elems[i] = debut;
    /* cherche la fin du mot */
    while (*debut && !isspace(*debut)) debut++; /* saute le mot */
    /* termine le mot par un \0 et passe au suivant */
    if (*debut) { *debut = 0; debut++; }
  }
  elems[i] = NULL;
}


int attend(pid_t pid) {
  while (1) {
    int status;
    int r = waitpid(pid,&status,0);
    if (r<0) {
      if (errno==EINTR) continue;
      perror("erreur de waitpid");
      return -1;
    }
    if (WIFEXITED(status)) {
      return (WEXITSTATUS(status));
    }
    return -1;
  }
}


void execute_fils (char **cmd_args) {
  execvp(cmd_args[0], /* programme à exécuter */
	 cmd_args     /* argv du programme à exécuter */
	 );
  perror("impossible d'éxecuter la commande");
  exit(1);
}

void execute(char **cmd_args) {
  pid_t pid;

  if (!cmd_args[0]) { return ; } /* ligne vide */

  if ( 0 == strcmp(cmd_args[0], "cd") ) {
    chdir(cmd_args[1]);
    return;
  }

  switch (pid = fork()) {
  case -1: {
    perror("fork a échoué");
    return ;
  }
  case 0: {
    execute_fils(cmd_args);
  }
  default: {
    attend(pid);
  }}
}


int main()
{
  while (1) {
    affiche_invite();
    lit_ligne();
    decoupe(ligne, elems, MAXELEMS);
    execute(elems);
  }
  return 0;
}
