#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

typedef void (*sighandler_t)(int);
char c = '\0';

void handle_signal(int signo)
{
	printf("\n[$LEASH] "); //On affiche le prompt
	fflush(stdout); //fflush s'assure que le Shell affiche bien le prompt directement même si le buffer n'est pas plein
}

int main(int argc, char *argv[], char *envp[])
{
	signal(SIGINT, SIG_IGN); //on ignore le CTRL-C...
	signal(SIGINT, handle_signal);//a la place on l'envoie au handler
	printf("[$LEASH] ");
	while(c != EOF) { //tant que l'on n'a pas appuyer sur CTRL-D
		c = getchar();
		if(c == '\n') //si l'on appuie sur Entree (peut etre une commande vide)...
			printf("[$LEASH] ");//... alors on affiche le prompt directement
	}
	printf("\n");
	return 0;
}
