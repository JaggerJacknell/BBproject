#include <stdlib.h>
#include <stdio.h>
#include  <sys/types.h>
#include  <unistd.h>

int main(){
	
	char * CommArgv[5]={"cat","file1.txt","||","cat","file2.txt"}; //C'est pour l'exemple
	
	int orFound=0;
	int i=0;
	
	while ((i<5)&&(orFound==0)) { //Changer 5 s'il faut
		if (!strcmp(CommArgv[i],"||")) {
			orFound=1;
			i--;
		}
		i++;
	}
	
	//printf("i=%d orFound=%d\n",i,orFound)
	
	if (orFound) {
		char * CommArgv1[i+1];
		int j;
		for (j=0; j<i;j++){
			CommArgv1[j]=CommArgv[j];
		}
		CommArgv1[i]=NULL;
		char * CommArgv2[5-i-1+1]; //Changer 5 s'il faut
		for (j=i+1; j<5;j++){ //Changer 5 s'il faut
			CommArgv2[j-i-1]=CommArgv[j];
		}
		CommArgv2[5-i-1]=NULL;
		
		pid_t child_pid;
		child_pid = fork ();
		if (child_pid != 0){
			// Nous sommes dans le processus parent.
			execvp(CommArgv1[0],CommArgv1);
			// On ne sort de la fonction execvp uniquement si une erreur survient.
			fprintf (stderr, "une erreur est survenue au sein de execvp\n");
			abort ();
		} else {
			// Nous sommes dans le processus fils.
			execvp(CommArgv2[0],CommArgv2);
			// On ne sort de la fonction execvp uniquement si une erreur survient.
			fprintf (stderr, "une erreur est survenue au sein de execvp\n");
			abort ();
		}
		
	}
	
}
