#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX 20
char *commandArgv[MAX];
int commandArgc = 0;

char buffer[50] = "ls cd \"cat pj\" grap sdf aze \"p lu sf\""; //C'est pour l'exemple

void separateCommand()
{
	//Separation des parties via un guillemet dans un tableau betweenQuotes
	char *betweenQuotes[MAX];
	int i=0;
	char* bufferPointer;												// on déclare un pointeur sur le buffer
	bufferPointer = strtok(buffer, "\"");								//on scinde le buffer en parties séparées par des guillemets
	while (bufferPointer != NULL) {										// tant que le pointeur n'est pas NULL (fin du buffer)
		betweenQuotes[i] = bufferPointer;								// on passe les parties du buffer dans le tableau betweenQuotes
		bufferPointer = strtok(NULL, "\"");								// on passe a la partie suivante
		i++;
	}
	//Separation des parties hors guillemets via les espaces, pas de séparation sinon
	int j;
	for (j=0;j<i;j++){
		if (j % 2 == 0){ //on est a l'exterieur des guillemets
			char* bufferPointer2;
			bufferPointer2 = strtok(betweenQuotes[j], " ");
			while (bufferPointer2 != NULL) {
				commandArgv[commandArgc] = bufferPointer2;
				bufferPointer2 = strtok(NULL, " ");
				commandArgc++;
			}
		} else { //on est a l'interieur de guillemets
			commandArgv[commandArgc] = betweenQuotes[j];
			commandArgc++;
		}
	}
		
}

int main() {
	separateCommand();
	/*int i; //C'est pour les tests
	for (i=0;i<commandArgc;i++){
		printf("%s\n",commandArgv[i]);
	}*/
}
