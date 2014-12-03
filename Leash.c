
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


/**
 *	Lecture des commandes de l'utilisateur
 */

void cleanCommand()  //cleanCommand permet de nettoyer le tableau des commandes
{                     //commandArgc est le nombre de commandes de l'utilisateur
                      //commandargv[] est le tableau qui contient les commandes de l'utilisateur
  while (commandArgc != 0) {    //tant que l'on a des commandes dans le tableau

         commandArgv[commandArgc] = NULL;  //on supprimme le pointeur sur la commande
         commandArgc--;                    // on enlève la commande du tableau
      }

  buffin = 0;                //on nettoie le buffer qui contient les chars rentrés par l'utilisateur

}
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


int existCommand()
{
  for(int j=0;j<MAX;j++) {
    if strcmp(commandArgv[0],strings[j]==0) {
      return 1;
    }
  }
}


void execCommand()
{
  pid_t pid;
  struct sigaction sig;
  /* Avant de lancer la commande dans le fork(), on va appeler sigaction.
  On va donc désactiver le CTRL-C.*/
  signal(SIGINT, SIG_IGN); //on ignore le CTRL-C...

  if (fork()<0) {
    perror("Le processus fils n'a pas été crée")
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

void wait_l(pid_t pid)
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

void handle_signal(int signo)
{
  printf("\n[$LEASH] "); //On affiche le prompt
  fflush(stdout); //fflush s'assure que le Shell affiche bien le prompt directement même si le buffer n'est pas plein
}
