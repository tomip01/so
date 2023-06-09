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

void ejecutar_hijo(int hijo, int start, int n, int pipe_ida[2], int pipe_vuelta[2], int pipe_nodos[][2]){
	// cierro pipes

	// printf("soy el hijo %i \n", hijo);
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
		if (i != anterior && i != hijo) {
			close(pipe_nodos[i][PIPE_READ]);
			close(pipe_nodos[i][PIPE_WRITE]);
		} else if (i == anterior) {
			close(pipe_nodos[anterior][PIPE_WRITE]);
		} else {
			close(pipe_nodos[i][PIPE_READ]);
		}
	}

	if (hijo == start) {
		int valor_inicio;
		read(pipe_ida[PIPE_READ], &valor_inicio, sizeof(valor_inicio));
		int valor_secreto; int actual;

		do {
			valor_secreto = generate_random_number();
		} while (valor_secreto <= valor_inicio);		// genero numeros random hasta que sean mayores que el valor de inicio
		actual = valor_inicio;

		printf("el num random es %i \n", valor_secreto);
		
		while (actual < valor_secreto) {
			// int anterior = (hijo-1) < 0 ? n-1 : hijo-1;	// me fijo para no pasarme de rango en los extremos
			int anterior;
			if (hijo-1 < 0)
				anterior = n-1;
			else
				anterior = hijo-1;
			write(pipe_nodos[hijo][PIPE_WRITE], &actual, sizeof(actual));
			read(pipe_nodos[anterior][PIPE_READ], &actual, sizeof(actual));
			actual++;
			printf("soy el hijo %i, y el actual es %i \n", hijo, actual);
		}

		int termino = -1;		// aviso que termino a los otros hijos
		write(pipe_nodos[hijo][PIPE_WRITE], &termino, sizeof(termino));

		// aviso al padre que termino, 
		write(pipe_vuelta[PIPE_WRITE], &actual, sizeof(actual));
		
	} else {
		int actual;
		printf("el actual es %i \n", actual);
		while (1) {
			int anterior;
			if (hijo-1 < 0)
				anterior = n-1;
			else
				anterior = hijo-1;
			// int anterior = (hijo-1) < 0 ? n-1 : hijo-1;
			ssize_t tmp = read(pipe_nodos[anterior][PIPE_READ], &actual, sizeof(actual));
			if (tmp == 0) printf ("falle \n");
			if (actual == -1){
				write(pipe_nodos[hijo][PIPE_WRITE], &actual, sizeof(actual));	
				break;			
			}
			actual++;
			write(pipe_nodos[hijo][PIPE_WRITE], &actual, sizeof(actual));
			printf("soy el hijo %i, y el actual es %i \n", hijo, actual);
		}
		
	}

	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{	
	if (argc != 4){ printf("Uso: anillo <n> <c> <s> \n"); exit(0);}
	//Funcion para cargar nueva semilla para el numero aleatorio
	srand(time(NULL));

	int status, pid, n, start, buffer;
	n = atoi(argv[1]);
	buffer = atoi(argv[2]);
	start = atoi(argv[3]);

    
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
		if (hijo == 0) ejecutar_hijo(i,start,n,pipe_ida,pipe_vuelta,pipe_nodos);
		else childrens[i] = hijo;
	}


	// cierro nodos entre hijos
	close(pipe_ida[PIPE_READ]);
	close(pipe_vuelta[PIPE_WRITE]);
    
	for (int i = 0; i < n; i++) {
		close(pipe_nodos[i][PIPE_READ]);
		close(pipe_nodos[i][PIPE_WRITE]);
	}

	// le digo que empiece al hijo start
	write(pipe_ida[PIPE_WRITE], &buffer, sizeof(buffer));

	// leo resultado
	int res;
	for (size_t i = 0; i < n; i++)
	{
		wait(NULL);	
	}
	
	read(pipe_vuelta[PIPE_READ], &res, sizeof(res));

	printf("El resultado es: %i \n",res);

	return 0;
}
