#include <unistd.h>
#include <stdio.h>
#include <errno.h>

   int main() {
       char cwd[1024];
       if (getcwd(cwd, sizeof(cwd)) != NULL)
           fprintf(stdout, "Répertoire de travail actuel: %s\n", cwd);
       else
           perror("getcwd() erreur");
       return 0;
   }
