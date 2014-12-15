#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

void rechercheAnd(){
	
	char * CommArgv[5]={"cat","file1.txt","&&","cat","file3.txt"}; //C'est pour l'exemple
	
	int andFound=0;
	int i=0;
	
	while ((i<5)&&(andFound==0)) {  //Changer 5 si on change le nombre de commande
		if (!strcmp(CommArgv[i],"&&")) {
			andFound=1;
			i--;
		}
		i++;
	}
	
	//printf("i=%d andFound=%d\n",i,andFound);
	
	if (andFound) {
		char * CommArgv1[i+1];
		int j;
		for (j=0; j<i;j++){
			CommArgv1[j]=CommArgv[j];
		}
		CommArgv1[i]=NULL;
		char * CommArgv2[5-i-1+1]; //Changer 5 s'il faut
		for (j=i+1; j<5;j++){
			CommArgv2[j-i-1]=CommArgv[j];
		}
		CommArgv2[5-i-1]=NULL;
		
		/*int l=0;
		while (CommArgv1[l]!=NULL) {
			printf("%s\n",CommArgv1[l]);
			l++;
		}*/
		
		if (fork() == 0){
			//do child stuff here
			execvp(CommArgv1[0],CommArgv1); /*since you want to return errno to parent do a simple exit call with the errno*/
			exit(errno);
		} else {
			//parent stuff
			int status;
			wait(&status);       /*you made a exit call in child you need to wait on exit status of child*/
			if(WIFEXITED(status))
			//printf("child exited with = %d\n",WEXITSTATUS(status));
			if (WEXITSTATUS(status)==0) {execvp(CommArgv2[0],CommArgv2);}
		}
	}
}

int main(){
	rechercheAnd();
}
