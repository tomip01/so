/*
	EJERCICIO 20:
	por enunciado asumo que existen: -N -M -FN -NC
*/

#include <signal.h> /* constantes como SIGINT*/
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

enum {READ, WRITE};
#define N 3
#define M 4
#define FN "archivo"
#define NC 8

void ejecutar_hijo(int hijo, int fd, int *pipe_out[][2], int *pipe_in[][2])
{
	for (int i = 0; i < NC; ++i)
	{
		// estos pipes no lo usa ningún hijo
		close(pipe_out[i][WRITE]);
		close(pipe_in[i][READ]);

		// cierro pipes entre hijos
		if (i != hijo)
		{
			close(pipe_out[i][READ]);
			close(pipe_in[i][WRITE]);
		}
	}

	int leer = 0;
	while (leer != 0) {
		int fila[M];
		cargar_fila(fd, &fila);

		int promedio = calcular_promedio(&fila);
		write(pipe_in[hijo][WRITE], &promedio, sizeof(promedio));
		read(pipe_out[hijo][READ], &leer, sizeof(leer));	
	}

	exit(0);
}

int main(int argc, char const *argv[])
{
	// abro pipes
	int pipe_out[NC][2];
	int pipe_in[NC][2];
	for (int i = 0; i < NC; ++i)
	{
		pipe(pipe_out[i]);
		pipe(pipe_in[i]);
	}

	int fd = open(FN);

	// creo hijos
	for (int i = 0; i < NC; ++i)
	{
		int hijo = fork();
		if ( hijo == 0)
			ejecutar_hijo(i,fd, pipe_out, pipe_in);
	}

	// cierro pipes que no uso
	for (int i = 0; i < NC; ++i)
	{
		close(pipe_out[i][READ]);
		close(pipe_in[i][WRITE]);
	}

	int promedios[N];
	int fila_actual = 0;
	while (fila_actual < N) {
		int minimo = min(N - fila_actual, NC);
		for (int i = 0; i < minimo; ++i)
		{
			int fila = fila_actual + i;
			write(pipe_out[i][WRITE], &fila, sizeof(fila));
		}
		for (int i = 0; i < minimo; ++i)
		{
			int fila = fila_actual + i;
			read(pipe_out[i][WRITE], &promedios[fila], sizeof(promedios[fila]));
		}
		fila_actual += minimo;
	}

	// aviso a los hjijos que corten
	for (int i = 0; i < NC; ++i)
	{
		int temp = -1;
		write(pipe_out[i][WRITE], &temp, sizeof(temp));
	}

	sort(&promedios);
	write(1, &promedios, sizeof(promedios));	// STDOUT es 1

	return 0;
}


/*
	EJERCICIO 21
*/

int termino = 0;

void sig_handler(int sig) {
	termino = 1;
}

void ejecutarNieto(int pipe_nieto[2][2]) {
	close(pipe_nieto[0][WRITE]);
	close(pipe_nieto[1][READ]);

	int numero;
	read(pipe_nieto[0][READ], &numero, sizeof(numero));
	int res = calcular(numero);

	pid_t padre = getppid();

	kill(padre, SIGURG);

	write(pipe_nieto[1][WRITE], &res, sizeof(res));
}

void ejecutarHijo(int hijo, int pipes[][2], int n) {
	for (int i = 0; i < N*2; ++i)
	{
		// estos pipes no lo usa ningún hijo
		close(pipes[i][WRITE]);
		close(pipes[n+i][READ]);

		// cierro pipes entre hijos
		if (i != hijo)
		{
			close(pipes[i][READ]);
			close(pipes[N*i][WRITE]);
		}
	}

	// creo pipe con nieto
	int pipe_nieto[2][2];
	pipe(pipe_nieto[0]);
	pipe(pipe_nieto[1]);

	// creo nieto
	int nieto = fork();
	if (nieto == 0){
		ejecutarNieto(pipe_nieto);
		return 0;
	}

	close(pipe_nieto[0][READ]);
	close(pipe_nieto[1][WRITE]);

	signal(SIGURG,sig_handler);

	int numero;
	read(pipes[hijo][READ], &numero, sizeof(numero));	// numero del padre
	write(pipe_nieto[0][WRITE], &numero, sizeof(numero)); // le paso al nieto el numero a calcular

	int resultado;
	// podría pasar que la interrupción llegue justo antes de mandar el mensaje al padre de que termino o no
	// ARREGLAR eso
	while(!termino) {
		if (termino) {
			read(pipe_nieto[1][READ],&resultado, sizeof(resultado));
			write(pipes[hijo+n][WRITE],&termino, sizeof(termino));
			write(pipes[hijo+n][WRITE],&numero, sizeof(numero));
			write(pipes[hijo+n][WRITE],&resultado, sizeof(resultado));
		}

		// aviso al padre
		read(pipes[hijo][READ],NULL, sizeof(int));
		write(pipes[hijo+n][WRITE],&termino, sizeof(termino));		
	}

}

