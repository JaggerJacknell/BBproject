#include <sys/stat.h>  //mkdir
#include <sys/types.h>
#include <stdio.h>  //perror
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAX 10

int main(int argc, char *argv[]) {

int i;
char* args[5];
args[0] = "tar";
args[1] = "xzvf";
args[2] = argv[1];
args[3] = "-C";
//args[4] = argv[2];
args[4] = NULL;

FILE *fp;
char *line = NULL;
char *result = NULL;
size_t len = 0;
ssize_t read;
char *strings[MAX]; //tableau dans lequel on va enregistrer les commandes possibles pour l'utilisateur



mode_t mask = umask(0); //on met le umask à 0 (pas de restrictions)
int result_code = mkdir("proji",0700); //on crée le répertoire
umask(mask); //on remet les droits initiaux

  if(result_code ==-1) {
	  perror(argv[0]);
	  printf("Erreur dans la création du répertoire \n");
	  exit(EXIT_FAILURE);
  }

  if(result_code==0){

    printf("Le répertoire a bien été crée \n");

    int fx = execv(args[0],args); //on exécute le programme tar pour décompresser le fichier

	if(fx==-1){
		printf("La décompression de l'archive n'a pas été possible\n");
	}

    fp = fopen("/home/kejji/proj/meta", "r"); //on ouvre le fichier à lire dans fp
    if (fp == NULL) {
              fprintf(stderr, "Erreur dans l'ouverture du fichier\n");
              exit(EXIT_FAILURE);
    }


    while ((read = getline(&line, &len, fp)) != -1) { //getline() fait un malloc automatique pour line
               printf("On récupère une ligne de taille %zu :\n", read);
               printf("%s", line);
               if(strchr (line,'#') != NULL) {

               }

               else if (strchr (line,'$') != NULL) {
                 strings[i] = strdup(line);
                 i++;

               }

               else if (strchr (line,'>') != NULL) {
                 result = strdup(line);
               }

           }

unlink("meta");
fclose(fp);
free(line);
free(result);
exit(EXIT_SUCCESS);

      }


return 0;
}
