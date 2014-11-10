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
{
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
  if strcmp(commandArgv[0],Listtemp[]==0)
  {
    return 0;
  }  
}
