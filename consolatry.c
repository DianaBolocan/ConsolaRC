#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/stat.h>
#include <time.h>

static char *fragment (char *str){
 char *other = malloc(strlen(str) + 1);
 if(other != NULL)
	strcpy(other,str);
 return other;
}

void mystat (char *msg,struct stat myStat){
 strcpy(msg,"Drepturi: ");
 if(S_ISDIR(myStat.st_mode))  {strcat(msg,"d");} else {strcat(msg,"-");}
 if(myStat.st_mode & S_IRUSR) {strcat(msg,"r");} else {strcat(msg,"-");}
 if(myStat.st_mode & S_IWUSR) {strcat(msg,"w");} else {strcat(msg,"-");}
 if(myStat.st_mode & S_IXUSR) {strcat(msg,"x");} else {strcat(msg,"-");}
 if(myStat.st_mode & S_IRGRP) {strcat(msg,"r");} else {strcat(msg,"-");}
 if(myStat.st_mode & S_IWGRP) {strcat(msg,"w");} else {strcat(msg,"-");}
 if(myStat.st_mode & S_IXGRP) {strcat(msg,"x");} else {strcat(msg,"-");}
 if(myStat.st_mode & S_IROTH) {strcat(msg,"r");} else {strcat(msg,"-");}
 if(myStat.st_mode & S_IWOTH) {strcat(msg,"w");} else {strcat(msg,"-");}
 if(myStat.st_mode & S_IXOTH) {strcat(msg,"x");} else {strcat(msg,"-");}
}

void myFind (char *msg,struct stat myStat){
 mystat(msg,myStat); strcat(msg,"\n");
 
 struct tm *time;
 
 time = localtime(&myStat.st_atime);
 strcat(msg,"Last Access: "); strcat(msg,asctime(time)); strcat(msg,"\n");

 time = localtime(&myStat.st_mtime);
 strcat(msg,"Last Modification: "); strcat(msg,asctime(time)); strcat(msg,"\n");

 time = localtime(&myStat.st_ctime);
 strcat(msg,"Last Status Change: "); strcat(msg,asctime(time)); strcat(msg,"\n");

}

int loginCheckup(char *user)
{
 FILE *fd;
 char *line = NULL;
 size_t n = 0;
 int verif = 0;
 
 fd = fopen("./usernames.txt","r");
 if(fd == NULL){
	printf("Eroare la open().\n"); 
	exit(0);
 }
 
 while(getline(&line,&n,fd) != -1){
	line[strcspn(line,"\n")] = '\0';
	if(strcmp(line,user) == 0){
		verif = 1;
		break;
	}
 }
 fclose(fd);
 return verif;
}

int main()
{
 int biti;
 char msg[1024];
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
		
		//socket parinte
 		close(socket[0]);
		if(write(socket[1],comanda,sizeof(comanda)) < 0) {printf("Write() Error.\n"); exit(1);}
			 
		//wait function
		if(wait(&status) < 0){
		       	printf("Wait() Error.\n");
			exit(2);
		}

		if(read(socket[1],msg,1025) < 0) {printf("Read() Error.\n"); exit(1);}
		
		if(msg != NULL)
			printf("%s\nDimensiune: %i bytes\n",msg,strlen(msg));
			

		close(socket[1]);	
	 }else{
		struct stat myStat;
		char msg[1024];
		char comanda[100];

		close(socket[1]);

		if(read(socket[0],comanda,101) < 0) 
			{printf("Read() Error.\n"); exit(1);}
		comanda[strcspn(comanda,"\n")] = '\0';

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
		
		if (strcmp(argv[0],"login:") == 0) {
			if(loginCheckup(argv[1]))
				strcpy(msg,"Welcome back!");
			else
				strcpy(msg,"Username not found!\n");
		} else if (strcmp(argv[0],"myfind") == 0){
			if(stat(argv[1],&myStat) < 0)
				strcpy(msg,"Stat() Error.");
			else
				myFind(msg,myStat);
		} else if (strcmp(argv[0],"mystat") == 0){
			if(stat(argv[1],&myStat) < 0)
				strcpy(msg,"Stat() Error.");
			else
				myFind(msg,myStat);
		} else  if(execvp(argv[0],argv) < 0){
			strcpy(msg,"Execvp() Error.");			
			exit(0);
		}
		
		if(write(socket[0],msg,sizeof(msg)) < 0) {printf("Write() Error.\n"); exit(1);}
		
		close(socket[0]);
		exit(0);
	 }
 }
}

