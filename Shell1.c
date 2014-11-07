#include <sys/stat.h>  //mkdir
#include <sys/types.h>
#include <stdio.h>  //perror
#include <stdlib.h>
#define MAX 10

int main(int argc, char *argv[]) {

char *args[3] = {"tar xzvf argv[0].tgz", "-C /home/amiri/Desktop/ProjetLeash/", NULL};
FILE *fp;
char *line = NULL;
size_t len = 0;
ssize_t read;
char *strings[MAX]; //tableau dans lequel on va enregistrer les commandes possibles pour l'utilisateur
int i=0;

mode_t mask = umask(0); //on met le umask à 0 (pas de restrictions)
int result_code = mkdir("/home/amiri/Desktop/ProjetLeash/", 0777); //on crée le répertoire
umask(mask); //on remet les droits initiaux

  if(result_code ==-1) {
	  perror(argv[0]);
	  printf("Erreur dans la création du répertoire/n");
	  exit(EXIT_FAILURE);
  }

  if(result_code==0){

    printf("Le répertoire a bien été crée");
    execvp(args[0],args); //on exécute le programme tar pour décompresser le fichier
    fp = fopen("/home/amiri/Desktop/ProjetLeash/", "r"); //on ouvre le fichier à lire dans fp
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

      free strings[i]; //incomplet (pour ne pas oublier)
      free(line);
      fclose(fp);
      exit(EXIT_SUCCESS);
      }


return 0;
}
