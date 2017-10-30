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
#include <fcntl.h>

static char *fragment (char *str){
 char *other = malloc(strlen(str) + 1);
 if(other != NULL)
	strcpy(other,str);
 return other;
}

void mystat (char *msg,struct stat myStat){
 strcpy(msg,"Access: ");
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

void myFind (char *msg,struct stat myStat,char *pth){
 mystat(msg,myStat); strcat(msg,"\n");
 
 struct tm *time;
 
 time = localtime(&myStat.st_atime);
 strcat(msg,"Last Access: "); strcat(msg,asctime(time)); 

 time = localtime(&myStat.st_mtime);
 strcat(msg,"Last Modification: "); strcat(msg,asctime(time)); 

 time = localtime(&myStat.st_ctime);
 strcat(msg,"Last Status Change: "); strcat(msg,asctime(time));

 char x[10];
 int y = (int)myStat.st_size;
 sprintf(x,"%d",y);
 strcat(msg,"Total Size: "); strcat(msg,x); strcat(msg,"\n");

 y = (int)myStat.st_blksize; sprintf(x,"%d",y);
 strcat(msg,"Blocksize for file system I/O: "); strcat(msg,x); strcat(msg,"\n");

 y = (int)myStat.st_dev; sprintf(x,"%d",y);
 strcat(msg,"Id of device containing the file: "); strcat(msg,x); strcat(msg,"\n");

 y = (int)myStat.st_ino; sprintf(x,"%d",y);
 strcat(msg,"Inode: "); strcat(msg,x); strcat(msg,"\n");

 y = (int)myStat.st_nlink; sprintf(x,"%d",y);
 strcat(msg,"Number of hard links: "); strcat (msg,x); strcat(msg,"\n");

 y = (int)myStat.st_uid; sprintf(x,"%d",y);
 strcat(msg,"User ID of owner: "); strcat(msg,x); strcat(msg,"\n");

 y = (int)myStat.st_gid; sprintf(x,"%d",y);
 strcat(msg,"Group ID of owner: "); strcat(msg,x); strcat(msg,"\n");

 y = (int)myStat.st_rdev; sprintf(x,"%d",y);
 strcat(msg,"Device ID: "); strcat(msg,x); strcat(msg,"\n");

 y = (int)myStat.st_blocks; sprintf(x,"%d",y);
 strcat(msg,"Number of blocks: "); strcat(msg,x); strcat(msg,"\n");

 char path[100];
 strcpy(path,pth);
 char actualpath[100];
 char *ptr;
 if (realpath(path,actualpath) == NULL)
	printf("Realpath() Error.\n");
 else{ strcat(msg,"Path: "); strcat(msg,actualpath);}

 //Waiting for xstat() to get date of birth (sorry)
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

void cd(char *path){
 char pth[100];
 if(chdir(path) < 0)
	printf("Chdir() Error.\n");
 else{
	getcwd(pth,sizeof(pth));
	printf("Success: %s\n",pth);
 }
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
 int set;

 printf("Login: ");
 if(fgets(comanda,sizeof(comanda),stdin) == NULL){
	printf("Expecting char array. Closing program...");
	exit(0);
 }

 comanda[strcspn(comanda,"\n")] = '\0';
 if(loginCheckup(comanda)) {printf("Welcome back!\n");} 
 else {printf("Unknown username. Closing program...\n"); exit(1);}

 while(1){
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, socket) < 0) { 
		printf("Socketpair() Error.\n"); 
		exit(0); 
	}

	if((fiu = fork()) < 0)
	{
		printf("Fork() Error.\n");
		exit(1);
	}
	else if (fiu)
	{
		printf("\n>");
		//citire line-by-line
		if(fgets(comanda,sizeof(comanda),stdin) == NULL || strcmp(comanda,"quit\n") == 0)
			exit(2);
		
 		if(close(socket[0]) < 0) {printf("Close() Error.\n"); exit(0);}
		if(write(socket[1],comanda,sizeof(comanda)) < 0) {printf("Write() Error.\n"); exit(3);}

		//wait function
		if(wait(&status) < 0){
		       	printf("Wait() Error.\n");
			exit(4);
		}
		
		if(read(socket[1],msg,1025) < 0) {printf("Read() Error.\n"); exit(5);}
		
		
		if(strlen(msg))
			printf("%s\nSize: %i bytes.\n",msg,strlen(msg));
			
		msg[0]='\0';
		if(close(socket[1]) < 0) { printf("Close() Error.\n"); exit(0);}	
	 }else{
		struct stat myStat;
		char msg[1024];
		char comanda[100];

		if(close(socket[1]) < 0) {printf("Close() Error\n"); exit(0);}
		if(read(socket[0],comanda,101) < 0) {printf("Read() Error.\n"); exit(6);}
		
		comanda[strcspn(comanda,"\n")] = '\0';

		//manual construction argv
		char *str = strtok(comanda," ");
		while(str != NULL) {
			argv[argc] = fragment(str);
			argc++;
			str = strtok(NULL," ");
		}
		argv[argc] = NULL;
		
		//checking input - divide in mystat, myfile and other
		if (strcmp(argv[0],"cd") == 0) {
			cd(argv[1]);		
		} else if (strcmp(argv[0],"myfind") == 0){
			if(stat(argv[1],&myStat) < 0)
				strcpy(msg,"Stat() Error.\n");
			else
				myFind(msg,myStat,argv[1]);
		} else if (strcmp(argv[0],"mystat") == 0){
			if(stat(argv[1],&myStat) < 0)
				strcpy(msg,"Stat() Error.\n");
			else
				mystat(msg,myStat);
		} else  if(execvp(argv[0],argv) < 0){
			strcpy(msg,"Execvp() Error.\n");			
		}
		
		if(write(socket[0],msg,sizeof(msg)) < 0) {printf("Write() Error.\n"); exit(7);}
		if(close(socket[0]) < 0) {printf("Close() Error.\n"); exit(0);}
		
		exit(8);
	 }
 }
}

