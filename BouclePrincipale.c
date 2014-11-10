int main()
{
    char *line = NULL;
    size_t size;

    Init();
    while(c != EOF) {                //tant que l'on n'a pas atteint EOF (gestion du CTRL-D)
    c = getchar();
    switch(c){
      case '\n':                    //si l'on appuie sur Entree (peut etre une commande vide)...
              printf("[$LEASH] ");  //... alors on affiche le prompt directement
              break;
      default:
                        getCommand();         // enregistre les commandes dans un tableau
                        existCommand();       // vérifie si la commande existe
                        execCommand();        // execute la commande
                        printf("\n[$LEASH] "); //On affiche le prompt
                        break;
                      }
                    }
    return 0;
}
