all: projetLeash

projetLeash: ProjetLeashFinal.o 
		gcc -o projetLeash ProjetLeashFinal.o -lreadline -lncurses

ProjetLeashFinal.o: ProjetLeashFinal.c headers2.h Tools.h
		gcc -o ProjetLeashFinal.o -c ProjetLeashFinal.c -lreadline -lncurses


clean:
	rm -f *.o core

mrproper: clean
	rm -f mon_executable
