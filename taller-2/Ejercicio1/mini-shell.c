#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include "constants.h"
#include "mini-shell-parser.c"

void ejecutar_hijo(int num_hijo, int count, int* pipes[][2], char **prog){
	if (num_hijo == count-1 && count > 1) {		
		close(*pipes[count-2][PIPE_WRITE]);
		for (int i = 0; i < count-2; i++) {
			close(*pipes[i][PIPE_READ]);
			close(*pipes[i][PIPE_WRITE]);
		}
		// la salida tiene que ir por stdout
		
	} else if (num_hijo == 0) {

		close(*pipes[0][PIPE_READ]);
		for (int i = 1; i < count-1; i++) {
			close(*pipes[i][PIPE_READ]);
			close(*pipes[i][PIPE_WRITE]);
		}

		// redirijo la salida al pipe
		dup2(*pipes[0][PIPE_WRITE], STD_INPUT);
		
	} else {
		for (int i = 0; i < count-1; i++) {
			// no cierro los pipes que me sirven
			if (i == num_hijo) {
				close(*pipes[i][PIPE_READ]);
			} else if (i == num_hijo-1){
				close(*pipes[i][PIPE_WRITE]);	
			} else {
				close(*pipes[i][PIPE_READ]);
				close(*pipes[i][PIPE_WRITE]);
			}
		}

		//redirijo entrada y salida estandar al pipe
		dup2(*pipes[num_hijo-1][PIPE_READ], STD_INPUT);
		dup2(*pipes[num_hijo][PIPE_WRITE], STD_OUTPUT);

	}
	execvp(prog[0],prog);
}

static int run(char ***progs, size_t count)
{	
	int r, status;

	//Reservo memoria para el arreglo de pids
	//TODO: Guardar el PID de cada proceso hijo creado en children[i]
	pid_t *children = malloc(sizeof(*children) * count);

	// creo pipes
	int pipes[count-1][2];
	for (int i = 0; i < count-1; i++) {
		pipe(pipes[i]);
	}
	
	// creo hijos
	for (int i = 0; i < count; i++) {
		pid_t hijo_i = fork();
		if (hijo_i == 0) ejecutar_hijo(i, count, &pipes, progs[i]);
		else children[i] = hijo_i;
	}
	

	//TODO: Pensar cuantos procesos necesito		=> 1 para cada programa a ejecutar 	=> count
	//TODO: Pensar cuantos pipes necesito.			=> 1 para conectar cada 2 proceso	=> count-1

	//TODO: Para cada proceso hijo:
			//1. Redireccionar los file descriptors adecuados al proceso
			//2. Ejecutar el programa correspondiente

	//Espero a los hijos y verifico el estado que terminaron
	for (int i = 0; i < count; i++) {
		waitpid(children[i], &status, 0);

		if (!WIFEXITED(status)) {
			fprintf(stderr, "proceso %d no terminó correctamente [%d]: ",
			    (int)children[i], WIFSIGNALED(status));
			perror("");
			return -1;
		}
	}
	r = 0;
	free(children);

	return r;
}


int main(int argc, char **argv)
{
	int programs_count;
	char*** programs_with_parameters = parse_input(argv, &programs_count);

	printf("status: %d\n", run(programs_with_parameters, programs_count));

	fflush(stdout);
	fflush(stderr);

	return 0;
}
