int main()
{

    size_t size;

    Init();
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

    for(i=0;i<6;i++){
      free(args[i]);
    }

    free(args);
    free(strings);
    free(line);
    free(result);
    return 0;
}
