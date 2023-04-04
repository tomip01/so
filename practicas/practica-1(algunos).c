/*
	EJERCICIO 20:
	por enunciado asumo que existen: -N -M -FN -NC
*/

void ejecutar_hijo(int hijo, int fd, int &pipe_out[][2], int &pipe_in[][2])
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
		pid_t hijo = fork();
		if ( hijo == 0)
			ejecutar_hijo(i,fd);
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
		int min = min(N - fila_actual, NC);
		for (int i = 0; i < min; ++i)
		{
			int fila = fila_actual + i;
			write(pipe_out[i][WRITE], &fila, sizeof(fila));
		}
		for (int i = 0; i < min; ++i)
		{
			int fila = fila_actual + i;
			read(pipe_out[i][WRITE], &promedios[fila], sizeof(promedios[fila]));
		}
		fila_actual += min;
	}

	// aviso a los hjijos que corten
	for (int i = 0; i < NC; ++i)
	{
		int temp = -1;
		write(pipe_out[i][WRITE], &temp, sizeof(temp));
	}

	sort(&promedios);
	write(STDOUT, &promedios, sizeof(promedios));

	return 0;
}

