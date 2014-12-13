#include <stdio.h>

void encode() {
	int clef = 67;
	
	FILE* fichier = NULL;
	
	fichier = fopen("test.txt", "r+");
	
	if (fichier != NULL){
		// Boucle de lecture des caractères un à un
		char caractereActuel;
        do {
			caractereActuel = fgetc(fichier); // On lit le caractère
			if (caractereActuel != EOF) {
				char nouveauCaractere = clef ^ caractereActuel;
				printf("%d\n",nouveauCaractere);
				fseek(fichier, -1, SEEK_CUR);
				fputc(nouveauCaractere, fichier);
			}
		} while (caractereActuel != EOF); // On continue tant que fgetc n'a pas retourné EOF (fin de fichier)
		
		fclose(fichier);
    }
}
