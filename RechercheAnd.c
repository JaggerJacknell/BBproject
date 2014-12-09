#include <stdlib.h>
#include <stdio.h>

int main(){
	
	//char * CommArgv[5]={"rrt","fg","sdcf","&&","fg"}; C'est pour l'exemple
	
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
		char * CommArgv1[i];
		int j;
		for (j=0; j<i;j++){
			CommArgv1[j]=CommArgv[j];
		}
		char * CommArgv2[5-i-1]; //Changer 5 s'il faut
		for (j=i+1; j<5;j++){
			CommArgv2[j-i-1]=CommArgv[j];
		}
		
		int result=1;
		
		if (execvp(CommArgv1)==0) {
			execvp(CommArgv2);
			result=0;
		}
		
		exit(result);
	}
	
}
