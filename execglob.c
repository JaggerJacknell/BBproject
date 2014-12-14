#include <string.h>
#include <stdio.h>
#include <glob.h> //essentiel pour le fonctionnement de glob

void execglob(char * argv[]){
	//on cherche le nombre de parametre et on place la commande et ses para dans un tableau
	char * commandBuffer[10]; //10 parce qu'il fallait mettre un entier coherent
	int nbrPara = 0;
	commandBuffer[0] = argv[0]; //on place la commande
	nbrPara++;
	while (argv[nbrPara][0]=='-'){ //on place les parametres
		commandBuffer[nbrPara] = argv[nbrPara];
		printf("%s\n",commandBuffer[nbrPara]);
		nbrPara++;	
	}
	//On peut maintenant utiliser glob(3)
	glob_t g;
	//on précise que les elements de n°0 à nbrPara de g.gl_pathv sont résevrés (pour la commande et ses parametres)
	g.gl_offs = nbrPara-1;
	//premiere commande en langage rationnel
	int paraPointer = nbrPara;
	if (argv[paraPointer]!=NULL){ //(si elle existe)
		glob(argv[paraPointer], GLOB_DOOFFS, NULL, &g);
		printf("1er %s\n",argv[paraPointer]);
		paraPointer++;
	}
	//autres commandes en langage rationnel
	while (argv[paraPointer]!=NULL){
		glob(argv[paraPointer], GLOB_DOOFFS | GLOB_APPEND, NULL, &g);
		printf("n-ieme %s\n",argv[paraPointer]);
		paraPointer++;
	}
	//on rentre la commande et ses parametres dans le tableau g.gl_pathv
	int i=0;
	while (i<nbrPara){
		printf("com mise %s\n",commandBuffer[i]);
		g.gl_pathv[i] = commandBuffer[i];
		i++;
	}
	execvp(commandBuffer[0], g.gl_pathv);
}

int main(){
	char * CommArgv[3]={"ls","*.c","*.jpg"}; //C'est pour l'exemple	
	execGlob(CommArgv);
}
