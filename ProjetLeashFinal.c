#include "headers.h"
#include "Tools.h"

/**
* Initialise l'écran du LEASH et affiche le prompt
*/

void Init()
{
    printf("\n-------------------------------------------------\n");
    printf("\tWelcome to LEASH ! Have Fun \n");
    printf("-------------------------------------------------\n");
    printf("\n\n");
    printf("\n[$LEASH] "); //On affiche le prompt
}

//Permet l'initialisation des fonctions readline (notamment pour l'historique des commandes et la fonction TAB)
initialize_readline ()
{

  rl_readline_name = "Leash";
  rl_attempted_completion_function = command_completion;
}

//permet de lire les commandes de l'utilisateur en utilisant la fonction getchar();
void getCommand()
{
        cleanCommand();                                                // nettoie le tableau des commandes
        while ((c != '\n') && (buffin < BUFFER_MAX_LENGTH)) {     //tant que l'on n'a pas appuyer sur Entree et que le buffer n'est pas plein...
                buffer[buffin++] = c;                             //...on stocke les char dans le buffer
                c = getchar();
        }
        buffer[buffin] = 0x00;                                    // lorsqu'on interrompt le stockage on met null en fin de buffer
        separateCommand();
}

//check si la fonction existe en comparant les commandes entrées par l'utilisateur avec le tableau des commandes du fichier meta
int existCommand()
{
  int j;
  for(j=0;j<MAX;j++) {
    if (strcmp(commandArgv[0],strings[j]==0)) {
      return 1;
    }
  }
  return 0;
}


//Cette fonction permet la gestion des pipes et des redirections
//Si l'on trouve un ">", "<", ou encore "|", on appelle la fonction associée

int processCommand() {

  int i;


  for(i=0;i<commandArgc; i++) {                  //on parcourt le tableau qui contient les commandes de l'utilisateur...
    if(strcmp(commandArgv[i], ">") == 0) {       //s'il contient la redirection ">"...
      return SuperiorCommand(i);                //...alors on lance la fonction SuperiorCommand();
    }
    else if(strcmp(commandArgv[i], "<") == 0) {  //s'il contient la redirection "<"...
      return InferiorCommand(i);                 //...alors on lance la fonction InferiorCommand();

    }
    else if(strcmp(commandArgv[i], "|") == 0) {   //s'il contient la redirection "|"...
        return PipeCommand(i);                    //...alors on lance la fonction PipeCommand();
    }
  }
  return excuteCommand();
}

//permet tout simplement d'exécuter les commandes rentrées par l'utilisateur
void execCommand()
{ int i;
  pid_t pid;
  struct sigaction sig;
  /* Avant de lancer la commande dans le fork(), on va appeler sigaction.
  On va donc désactiver le CTRL-C.*/
  signal(SIGINT, SIG_IGN); //on ignore le CTRL-C...

  if (fork()<0) {
    perror("Le processus fils n'a pas été crée");
  }
  if (fork()==0) {                            //lorsqu'on se trouve dans le fils...
    signal(SIGINT, SIG_DFL);                  //on réactive le CTRL-C
    signal(SIGINT, handle_signal);            //on l'envoie au handler
    if (existCommand()==1){                   //si la commande passée par l'utilisateur est permise...
      i = execvp(commandArgv[0],commandArgv); //...alors on l'exécute
      add_history(commandArgv[0]);             //et on l'ajoute à l'historique des commandes

      if(i<0){                                //sinon, si la commande n'existe pas
        printf("%s : %s\n", commandArgv[0], "La commande n'existe pas ou n'est pas permise");
        exit(1);
      }
    }

} else {
  wait_l(pid);
  signal(SIGINT, SIG_DFL); //on réactive le CTRL-C
  signal(SIGINT, handle_signal);//on l'envoie au handler
}
}

/*
 ============================================================================
	Méthode main() du programme
 ============================================================================
 */



int main(int argc, char* argv[])
{



    Init();
    mode_t mask = umask(0); //on met le umask à 0 (pas de restrictions)
    int result_code = mkdir("ProjetLeash", 0777); //on crée le répertoire
    umask(mask); //on remet les droits initiaux

      if(result_code ==-1) {
        perror(argv[0]);
        printf("Erreur dans la création du répertoire \n");
        exit(EXIT_FAILURE);
      }

      if(result_code==0){

        printf("Le répertoire a bien été crée \n");

        int fx = execvp(args[0],args); //on exécute le programme tar pour décompresser le fichier

        if(fx==-1){
        printf("La décompression de l'archive n'a pas été possible\n");
      }

        fp = fopen("/home/amiri/Desktop/ProjetLeash/meta", "r"); //on ouvre le fichier à lire dans fp
        if (fp == NULL) {
                  fprintf(stderr, "Erreur dans l'ouverture du fichier\n");
                  exit(EXIT_FAILURE);
        }


        while ((read = getline(&line, &len, fp)) != -1) { //getline() fait un malloc automatique pour line

                   if(strchr (line,'#') != NULL) {

                   }

                   else if (strchr (line,'$') != NULL) {
                     strings[i] = strdup(line+2);
                     i++;

                   }

                   else if (strchr (line,'>') != NULL) {
                     result = strdup(line+2);
                   }

               }

    unlink("meta");
    fclose(fp);
    while(TRUE) {


      while(c != EOF) {                //tant que l'on n'a pas atteint EOF (gestion du CTRL-D)
      c = getchar();
      switch(c){
      case '\n':                    //si l'on appuie sur Entree (commande vide)...
              printf("[$LEASH] ");  //... alors on affiche le prompt directement
              break;
      default:
                        initialize_readline();//on initialise le compléteur de commande
                        getCommand();         // enregistre les commandes dans un tableau
                        existCommand();       // vérifie si la commande existe
                        processCommand();     // on check si le tableau contient des redirections ou pipes
                        execCommand();        // execute la commande
                        printf("\n[$LEASH] "); //On affiche le prompt
                        break;
                      }
                    }
                  }

    for(i=0;i<6;i++){
      free(args[i]);
    }

    free(args);
    free(strings);
    free(line);
    free(result);
}
    return 0;
}
