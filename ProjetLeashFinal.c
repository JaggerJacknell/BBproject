#include "headers2.h"
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
    
}

//Permet l'initialisation des fonctions readline (notamment pour l'historique des commandes et la fonction TAB)
void initialize_readline ()
{

  rl_readline_name = "Leash";
  rl_attempted_completion_function = command_completion;
  

}

//permet de lire les commandes de l'utilisateur en utilisant la fonction getchar();
void getCommand()
{		
		 
  //cleanCommand();                // nettoie le tableau des commandes
  lineRead = readline("\n[$LEASH]");
		
	rl_bind_key ('\t', rl_complete);
	
			
  if (lineRead && *lineRead){
		add_history (lineRead);
        }
        
  strcpy(cmdLine, lineRead);                                 
        
	
}


void handleUserCommand()
{
        if (processCommand() == 0) { //si la commande n'est pas une built-in commande, alors on lance ça en foreground
                if(existCommand() == 1) {
                    execCommand(result);
                  }

                else {

                  printf("Les seules commandes autorisées sont :\n");
                  for (i = 0; strings[i] != '\0'; i++) {
                      printf("%d : %s   \n", i, strings[i]);
                }
        }
    }
}


//Cette fonction permet la gestion des pipes et des redirections
//Si l'on trouve un ">", "<", ou encore "|", on appelle la fonction associée
/**
 * built-in commands: exit, in, out, bg, fg, jobs, kill
 * 
 */

int processCommand() {


  for(i=0;i<MAX-1; i++) {                  //on parcourt le tableau qui contient les commandes de l'utilisateur...
    if(strcmp(commandArgv[i], ">") == 0) {       //s'il contient la redirection ">"...
      SuperiorCommand(i);                //...alors on lance la fonction SuperiorCommand();
      return 1;
    } 
    else if(strcmp(commandArgv[i], "<") == 0) {  //s'il contient la redirection "<"...
      InferiorCommand(i);                 //...alors on lance la fonction InferiorCommand();
	  return 1;
    }
    else if(strcmp(commandArgv[i], "|") == 0) {   //s'il contient la redirection "|"...
      PipedCommand(i);                    //...alors on lance la fonction PipeCommand();
	  return 1;
    }
    else if(strcmp(commandArgv[i], "fg") == 0) {   //s'il contient le signe "fg"...
        if (commandArgv[1] == NULL)
          return 0;

        int jobId = (int) atoi(commandArgv[1]);
        t_job* job = getJob(jobId, BY_JOB_ID);

        if (job == NULL)
          return 0;

        if (job->status == SUSPENDED || job->status == WAITING_INPUT)
          putJobForeground(job, TRUE);
        else                                                  
          putJobForeground(job, FALSE);
        return 1;                    
    }
    else if(strcmp(commandArgv[i], "bg") == 0) {   //s'il contient le signe "bg"...
        if (commandArgv[1] == NULL)
          return 0;

        else
          launchJob(commandArgv + 1, 0, BACKGROUND);
        return 1;
    }
    else if(strcmp(commandArgv[i], "jobs") == 0) {   //s'il contient le signe "jobs"...
        printJobs();
        return 1;
    }
    else if(strcmp(commandArgv[i], "kill") == 0) {   //s'il contient le signe "kill"...
        if (commandArgv[1] == NULL)
          return 0;
        killJob(atoi(commandArgv[1]));
        return 1;
    }
    else if (strcmp(commandArgv[i], "cd") == 0) {

        handleCdCommand();
        continue;
        return 1;
    }
    else if (strcmp(commandArgv[i], "exit") == 0) {
         exit(EXIT_SUCCESS);
    }
    else if (strcmp(commandArgv[i], "pwd") == 0) {
		handlePwdCommand();
        continue;
        return 1;
	}

  return 0;
}
}

/*
 ============================================================================
	Méthode main() du programme
 ============================================================================
 */


int main(int argc, char* argv[])
{	

	args[0] = "tar";
	args[1] = "xzvf";
	args[2] = argv[1];
	args[3] = "-C";
	args[4] = "./ProjetLeash";
	args[5] = NULL;


    Init();

    if(!(folderExists("ProjetLeash"))) {

      mode_t mask = umask(0); //on met le umask à 0 (pas de restrictions)
      result_code = mkdir("ProjetLeash", 0777); //on crée le répertoire
      umask(mask); //on remet les droits initiaux
    } else {

      result_code = 0;
    }
    
    switch (result_code) {

      case -1 : 
        perror(argv[0]);
        printf("Erreur dans la création du répertoire \n");
        exit(EXIT_FAILURE);
        break;
        
      case 0 :
      
        printf("Le répertoire a bien été crée \n");

        if((pid = fork()) < 0) {

          printf("Erreur dans le fork\n");
          exit(1);
	  }
        
        else if (pid == 0) {

          int fx = execvp(args[0],args); //on exécute le programme tar pour décompresser le fichier
          if(fx<0) {
            printf("La décompression de l'archive n'a pas été possible\n");
            exit(1);
          }

        } else {
          while (wait(&status) != pid)
            ;
        }

        if (changeDir("./ProjetLeash/") != 0) {
          printf("Erreur d'accès au répertoire\n");
          exit(1);
          break;
        }

        getcwd(rwd,sizeof(rwd));


        fp = fopen("./meta", "r"); //on ouvre le fichier à lire dans fp
        if (fp == NULL) {
                  fprintf(stderr, "Erreur dans l'ouverture du fichier\n");
                  exit(EXIT_FAILURE);
        }


        while ((reado = getline(&line, &lenth, fp)) != -1) { //getline() fait un malloc automatique pour line

                   if(strchr (line,'#') != NULL) {

                   }

                   else if (strchr (line,'$') != NULL) {
                     strings[i] = strdup(line+2);
                     strings[i] = strtok(strings[i], "\n");
                     i++;

                   }

                   else if (strchr (line,'>') != NULL) {
                     result = strdup(line+2);
                   }

               }

    
    unlink("./meta");
    fclose(fp);
    free(line);
    
    break;

    
default :
    
    break;
    
    
  }
    
  while(TRUE) {
		
	
    initialize_readline();//on initialise le compléteur de commande
    getCommand();         // enregistre les commandes dans un tableau
    separateCommand();
    handleUserCommand();
    
}
    for(i=0;i<6;i++){
      free(args[i]);
    }

    return 0;
}
