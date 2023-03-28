#include <signal.h> /* constantes como SIGINT*/
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int iteraciones = 0;

void sigurg_handler(int sig) {
	printf("ya va!\n");
	iteraciones++;
}

void sigint_handler(int sig) {
	wait(NULL);
	exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
  	pid_t pid_hijo = fork();

	if (pid_hijo == 0) {
		signal(SIGURG, sigurg_handler);

		while (iteraciones < 5) {}

		pid_t padre = getppid();
		kill(padre, SIGINT);
		execvp(argv[1], argv+1);

	} else {

		signal(SIGINT, sigint_handler);

		while (1) {
			kill(pid_hijo, SIGURG);
			sleep(1);
			printf("sup!\n");	
		}		
	}

	return 0;
}
