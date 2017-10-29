#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/un.h>
#include <sys/socket.h>

static char *fragment (char *str){
 char *other = malloc(strlen(str) + 1);
 if(other != NULL)
	strcpy(other,str);
 return other;
}

int main()
{
 char comanda[100];
 char *argv[100];
 int argc = 0;
 pid_t fiu;
 int status;
 int socket[2];
 int flag = 1;

 while(flag){
	printf("%i\n",flag);
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, socket) < 0) 
	{ 
		perror("Err... socketpair\n"); 
		exit(1); 
	}

	if((fiu = fork()) < 0)
	{
		printf("Eroare la fork().\n");
		exit(1);
	}
	else if (fiu)
	{
		//citire line-by-line
		if(fgets(comanda,sizeof(comanda),stdin) != NULL)
			flag = 0;
		comanda[strcspn(comanda,"\n")] = '\0';
		
		if(strcmp(comanda,"quit") == 0)
			exit(0);
		
		//socket parinte
 		close(socket[0]);
		if(write(socket[1],comanda,sizeof(comanda)) < 0) {printf("Eroare la write in socket.\n"); exit(1);}
			 
		//wait function
		if(wait(&status) < 0){
		       	printf("Eroare la wait().\n");
			exit(2);
		}
		close(socket[1]);	
	 }else{
		
		char comanda[1024];
		printf("Sunt in fiu.\n");

		close(socket[1]);
		if(read(socket[0],comanda,sizeof(comanda)) < 0) {printf("Eroare la read in socket.\n"); exit(2);}

		if(strcmp(comanda,"quit") == 0)
			exit(0);

		//manual construction argv
		char *str = strtok(comanda," ");
		while(str != NULL) {
			argv[argc] = fragment(str);
			argc++;
			str = strtok(NULL," ");
		}
		argv[argc] = NULL;
		
		if(execvp(argv[0],argv) < 0){
			printf("Eroare la executie.\n");
		}

		close(socket[0]);
	 }
}
}

