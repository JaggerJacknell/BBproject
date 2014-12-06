void changeDirectory()
{
        if (commandArgv[1] == NULL) {   //si le répertoire de destination n'est pas défini...
                chdir(getenv("HOME"));  //...le répertoire personnel de l'utilisateur est choisi (getenv renvoie un pointeur sur HOME)
        } else {
                if (chdir(commandArgv[1]) == -1) {    //en cas d'erreur dans la création du répertoire
                        printf(" Le répertoire %s n'existe pas\n", commandArgv[1]);
                }
                else {
                  chdir(commandArgv[1]);
                }
            }
        }
}

//ligne pour apeller la fonction (quelque part dans le main)
if (strcmp("cd", commandArgv[0]) == 0) {
                changeDirectory();
              }
