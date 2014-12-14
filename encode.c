#include <stdio.h>

void encode(char * name) { //mettre le nom du fichier
	int clef = 67;
	
	FILE* fichier = NULL;
	
	fichier = fopen(name, "r+");
	
	if (fichier != NULL){
		// Boucle de lecture des caractères un à un
		char caractereActuel;
        do {
			caractereActuel = fgetc(fichier); // On lit le caractère
			if (caractereActuel != EOF) {
				char nouveauCaractere = clef ^ caractereActuel;
				//printf("%d\n",nouveauCaractere); c'était juste pour verifier ce qu'il ecrivait
				fseek(fichier, -1, SEEK_CUR);
				fputc(nouveauCaractere, fichier);
			}
		} while (caractereActuel != EOF); // On continue tant que fgetc n'a pas retourné EOF (fin de fichier)
		
		fclose(fichier);
	}
}

int main(){
	encode("test.txt");	
}
