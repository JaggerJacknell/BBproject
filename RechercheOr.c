#include <stdlib.h>
#include <stdio.h>

int main(){
	
	//char * CommArgv[5]={"rrt","fg","||","ttt","fg"}; C'est pour l'exemple
	
	int orFound=0;
	int i=0;
	
	while ((i<5)&&(orFound==0)) { //Changer 5 s'il faut
		if (!strcmp(CommArgv[i],"||")) {
			orFound=1;
			i--;
		}
		i++;
	}
	
	//printf("i=%d orFound=%d\n",i,orFound);
	
	if (orFound) {
		char * CommArgv1[i];
		int j;
		for (j=0; j<i;j++){
			CommArgv1[j]=CommArgv[j];
		}
		char * CommArgv2[5-i-1]; //Changer 5 s'il faut
		for (j=i+1; j<5;j++){ //Changer 5 s'il faut
			CommArgv2[j-i-1]=CommArgv[j];
		}
		
		int result = 1;
		
		if (execvp(CommArgv1)==0) {result = 0;};
		if (execvp(CommArgv2)==0) {result = 0;};
		
		exit(result);		
	}
	
}
