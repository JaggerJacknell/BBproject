#include <readline/readline.h>
#include <readline/history.h>

static char *commandArray[] = {"login", "shutdown", "halt", "reboot", "install", "mount",
		                        "umount", "chsh", "exit", "last", "file", "mkdir", "grep",
                           "dd", "find", "mv", "ls", "dif", "cat", "ln", "df", "top",
                           "free", "quota", "at", "lp", "adduser", "groupadd", "kill",
                           "kill", "crontab", "ifconfig", "ip", "ping", "netstat",
                           "telnet", "ftp", "route", "rlogin", "rcp", "finger", "mail",
                           "nslookup", "passwd", "su", "umask", "chgrp", "chmod","chown",
                           "chattr", "sudo", "ps", "tar", "unzip", "gunzip", "unarj",
                           "mtools", "man", "unendcode", "uudecode", "which", "who", "echo"};

     char *command_generator ();
     char **command_completion ();


     initialize_readline ()
     {
       /* Allow conditional parsing of the ~/.inputrc file. */
       rl_readline_name = "Leash";

       /* Tell the completer that we want a crack first. */
       rl_attempted_completion_function = command_completion;
     }

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


       while (name = commandArray[list_index])  //tant que la commande entrée par l'utilisateur correspond à une commande du tableau des commandes autorisées
         {
           list_index++;                        //on incrémente l'index

           if (strncmp (name, text, len) == 0) //lorsque la commande entrée par l'utilisateur correspond entièrement à la commande autorisée...
             return (strdup(name));             //...on retourne une copie
         }


       return ((char *)NULL);                   //s'il n'y a pas de correspondances avec une commande autorisée alors on retourne NULL
     }
