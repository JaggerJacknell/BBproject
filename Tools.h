
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

  buffin = 0;                //on nettoie le buffer qui contient les chars rentrés par l'utilisateur

}


void separateCommand()
{
        char* bufferPointer;                                            // on déclare un pointeur sur le buffer
        bufferPointer = strtok(buffer, " ");                            //on scinde le buffer en commandes séparées par des espaces ex :{cat, README, NULL} et on pointe sur la 1ere commande (ici cat)
        while (bufferPointer != NULL) {                                 // tant que le pointeur n'est pas NULL (fin du buffer)
                commandArgv[commandArgc] = bufferPointer;                // on passe les commandes du buffer dans le tableau des commandes
                bufferPointer = strtok(NULL, " ");                       //on passe à la 2nde commande (ici README). Lors du second appel de strtok sur la meme chaine, le paramètre est NULL(convention)
                commandArgc++;                                           //ainsi de suite jusqu'à tomber sur NULL (fin du buffer), on stocke dans le tableau...
        }
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

char **command_completion (char *text, int start, int end) { //permet de compléter la commande et de la retourner

  char **matches = NULL; //on initialise les correspondances à NULL


  /* Si le mot à compléter est en début de ligne, alors c'est une commande.
     Autrement, c'est le nom d'un fichier dans le répertoire courant. */

  if (start == 0)
    matches = rl_completion_matches (text, command_generator);   //on complète la commande

  return (matches); //puis on la retourne
}

/* state permet de savoir si l'on doit commencer la recherche du mot depuis le début
(si state == 0, alors on doit commencer la recherche depuis le début du mot)*/

char *command_generator (char *text, int state) {

  int list_index;  //position actuelle dans le mot à compléter
  int len;         //longueur du mot à compléter
  const char *name;//nom de la commande qui correspond à ce que l'utilisateur rentre


  if (!state)                             //si state = 0 (c'est un nouveau mot)...
    {
      list_index = 0;                     //...alors on initialise l'index à 0 (on part du début du tableau)
      len = strlen (text);                //on enregistre la longueur du mot dans la variable len
    }


  while (name = (commandArray[list_index]))  //tant que la commande entrée par l'utilisateur correspond à une commande du tableau des commandes autorisées
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

    if(file == -1) { //si le fichier ne s'ouvre pas...
            printf("Le fichier de redirection ne peut pas s'ouvrir ou n'existe pas");
            exit(1); } //...on affiche un message d'erreur

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


  switch(fork()) {
  case -1:
    perror("fork()");
    exit(1);
  case 0:
        close(fd[0]);  /* the other side of the pipe */
        dup2(fd[1], 1);  /* automatically closes previous fd 1 */
        close(fd[1]);  /* cleanup */

      FILE * file2;
      char mystring;

      file2 = fopen (filename , "r");

      if (file2 == NULL) {
        perror ("Le fichier de redirection ne peut pas s'ouvrir ou n'existe pas"); }
      else {

     while ((mystring=fgetc(pFile)) != EOF) {
         putchar(mystring); /* print the character */
       }
        fclose (File2);
      }
     exit(0);

    default:

        close(fd[1]);  /* the other side of the pipe */
        dup2(fd[0], 0);  /* automatically closes previous fd 0 */
        close(fd[0]);  /* cleanup */

        execvp(argv[0], argv);


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

      default: //on doit exécuter la dernière commande du pipe dans le père car c'est celui-çi qui devra être écouté par le SHELL (et attendre la fin du processus A)
        close(fd[1]);
        dup2(fd[0], 0);
        close(fd[0]);

        execvp(argvB[0], argvB);


  }
 }
}
