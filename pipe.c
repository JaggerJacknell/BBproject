//Cette fonction permet la gestion des pipes et des redirections
//Si l'on trouve un ">", "<", ou encore "|", on appelle la fonction associée

int processCommand() {

	int i;

	for(i=0;i<commandArgc; i++) {                  //on parcourt le tableau qui contient les commandes de l'utilisateur...
		if(strcmp(commandArgv[i], ">") == 0) {       //s'il contient la redirection ">"...
			return SuperiorCommand(i);           //...alors on lance la fonction SuperiorCommand();
		}
		else if(strcmp(commandArgv[i], "<") == 0) {  //s'il contient la redirection "<"...
			return InferiorCommand(i);             //...alors on lance la fonction InferiorCommand();

		}
		else if(strcmp(commandArgv[i], "|") == 0) {   //s'il contient la redirection "|"...
		    return PipeCommand(i);             //...alors on lance la fonction PipeCommand();
		}
	}
	return excuteCommand();
}


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
            printf("Le fichier de redirection ne peut pas s'ouvrir ou n'existe pas")
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
				perror ("Le fichier de redirection ne peut pas s'ouvrir ou n'existe pas");
 	   else {

 		while ((mystring=fgetc(File2)) != EOF) {
 				putchar(mystring); /* print the character */
 			}
 	     fclose (File2);
 	   }
 	  exit(EXIT_SUCCESS);

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

	argvB[jj]=NULL; //on met NULL à la fin du tableau


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
