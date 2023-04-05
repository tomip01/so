#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include "constants.h"

int generate_random_number(){
	return (rand() % 50);
}

void ejecutar_hijo(int hijo, int start, int n, int *pipe_ida[][2], int *pipe_vuelta[][2], int *pipe_nodos[][2]){
	// cierro pipes
	if (hijo != start) {
		close(pipe_ida[PIPE_READ]);
		close(pipe_ida[PIPE_WRITE]);
		close(pipe_vuelta[PIPE_READ]);
		close(pipe_vuelta[PIPE_WRITE]);
	} else {
		close(pipe_ida[PIPE_WRITE]);
		close(pipe_vuelta[PIPE_READ]);
	}
	for (int i = 0; i < n; i++) {
		// cierro pipes entre nodos
		int anterior = (i-1) < 0 ? n-1 : i-1;
		close(pipe_nodos[anterior][PIPE_WRITE]);
		close(pipe_nodos[i][PIPE_READ]);
	}

	int empiezo;
	read(pipe_ida[hijo][PIPE_READ], &empiezo, sizeof(empiezo));
	if (empiezo != 0) {
		int rand; int actual;

		do {
			rand = generate_random_number();
		} while (rand >= empiezo);
		actual = rand;		
		
		while (actual < rand) {
			int anterior = (hijo-1) < 0 ? n-1 : hijo-1;
			write(pipe_nodos[hijo][PIPE_WRITE], &actual, sizeof(actual));
			read(pipe_nodos[anterior][PIPE_READ], &actual, sizeof(actual));
			actual++;
		}

		write(pipe_vuelta[hijo][PIPE_WRITE], &actual, sizeof(actual));
		
	} else {
		int actual;
		while ()
	}
}

int main(int argc, char **argv)
{	
	//Funcion para cargar nueva semilla para el numero aleatorio
	srand(time(NULL));

	int status, pid, n, start, buffer;
	n = atoi(argv[1]);
	buffer = atoi(argv[2]);
	start = atoi(argv[3]);

	if (argc != 4){ printf("Uso: anillo <n> <c> <s> \n"); exit(0);}
    
  	/* COMPLETAR */
    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i \n", n, buffer, start);
    
    /* COMPLETAR */

	int pipe_ida[2];
	int pipe_vuelta[2];
	int pipe_nodos[n][2];

	pipe(pipe_ida);
	pipe(pipe_vuelta);
	for (int i = 0; i < n; i++) {
		pipe(pipe_nodos[i]);
	}
	
	pid_t childrens[n];
	for (int i = 0; i < n; ++i) {
		pid_t hijo = fork();
		if (hijo == 0) ejecutar_hijo(i,n,&pipe_ida,&pipe_vuelta,&pipe_nodos);
		else childrens[i] = hijo;
	}
    
	// cierro nodos entre hijos
	for (int i = 0; i < n; i++) {
		close(pipe_nodos[i][PIPE_READ]);
		close(pipe_nodos[i][PIPE_WRITE]);
	}

	// le digo que empiece al hijo start
	write(pipe_ida[PIPE_WRITE], &buffer, sizeof(buffer));
	
	// leo resultado
	int res;
	read(pipe_vuelta[PIPE_READ], &res, sizeof(res));

	printf("El resultado es: %i",res);

	return 0;
}
