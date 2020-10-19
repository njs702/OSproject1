#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXLINE 4096
#define STDOUT_FILENO 1
#define FIFO1 "/tmp/fifo.1"
#define FIFO2 "/tmp/fifo.2"
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

void client(int,int), server(int,int);

int main(int argc, char *argv[]){
	int readfd, writefd;
	pid_t childpid;
	
	if((mkfifo(FIFO1, FILE_MODE) < 0) && (errno != EEXIST)){
		printf("can't create %s", FIFO1);
		exit(1);
	}
	else{
		printf("%s create completed!\n",FIFO1);
	}

	if((mkfifo(FIFO2, FILE_MODE) < 0) && (errno != EEXIST)){
		unlink(FIFO1);
		printf("can't create %s",FIFO2);
		exit(1);
	}
	else{
		printf("%s create completed!\n",FIFO2);
	}

	while(1){
	if((childpid=fork())==0){
		readfd = open(FIFO1, O_RDONLY,0);
		writefd = open(FIFO2, O_WRONLY,0);

		server(readfd,writefd);
		exit(0);
	}

	writefd = open(FIFO1, O_WRONLY,0);
	readfd = open(FIFO2, O_RDONLY,0);

	client(readfd, writefd);
	waitpid(childpid, NULL, 0);
	}
	close(readfd);
	close(writefd);
	unlink(FIFO1);
	unlink(FIFO2);
	exit(0);
	
}

void client(int readfd,int writefd){
	size_t len;
	size_t n;
	char buff[MAXLINE];
	char recieve[MAXLINE];
	char transmit[MAXLINE];
	char temp[MAXLINE];
	char file_name[MAXLINE];
	int i=2;
	int j=0;
	int count=2;
	
	printf("\nPlease enter\n");
	printf("1.read : file_name,r,bytes\n");
	printf("2.write : file_name,w,contents : ");

	fgets(buff, MAXLINE, stdin);

	
	len = strlen(buff);
	if(buff[len-1] == '\n')
		len--;

	write(writefd,buff,len);
	
	/*while(len == 0){
		len = read(readfd,recieve,MAXLINE);
	}
	recieve[len] = '\0';
	printf("Client : Server's response : %s",recieve);*/

	while((n=read(readfd,buff,MAXLINE))>0)
		write(STDOUT_FILENO,buff,n);
	
	
}

void server(int readfd,int writefd){
	int fd;
	size_t n;
	char buff[MAXLINE+1];
	FILE *fp = NULL;
	int fb;
	char recieve[MAXLINE];
	char transmit[MAXLINE];
	char access_type = '0';
	char file_name[MAXLINE];
	char file_contents[MAXLINE];
	char file_bytes[8];
	char temp[MAXLINE];
	char temp_buff[MAXLINE];
	char fstr[MAXLINE];
	char fstw[MAXLINE];
	int i,j,len=0 , byte = 0;
	int k;
	int a=0,b=2;
	pid_t child_process;
	

	file_name[0]='.';
	file_name[1]='/';
	
	

	
	if((n=read(readfd, buff, MAXLINE)) == 0){
		printf("end-of-file");
		exit(0);
	}
	buff[n] = '\0';
	
	for(a=0,b=2;buff[a]!=',';a++,b++){
		file_name[b]=buff[a];
	}
	file_name[b]='\0';
	access_type = buff[a+1];
	printf("access type is %c\n",access_type);
	
	child_process = fork();
	if(child_process > getpid()){
	if(access_type == 'r'){
		fp=open(file_name,O_RDONLY);
		fb = strlen(buff); // 문자열 길이
		for(b=0,a=a+3;a<fb;a++,b++){
			file_bytes[b] = buff[a];
		}
		fb = atoi(file_bytes);
		len=read(fp,fstr,fb);
		
		close(fp);
		fstr[len]='\0';
		printf("%d bytes 만큼의 문자열을 읽습니다...\n",fb);
		write(writefd,fstr,fb);
		close(fd);
	}
	if(access_type == 'w'){
		fb = strlen(buff);
		for(b=0,a=a+3;buff[a]!=EOF;a++,b++){
			fstw[b] = buff[a];
		}
		fp=open(file_name,O_WRONLY | O_APPEND);
		if(fp == -1){
			perror("No file\n");
			exit(0);
		}
		else{
			write(fp,fstw,strlen(fstw));
			close(fp);
			fb = strlen(fstw);
			sprintf(file_bytes,"입력한 byte 수 : %d\n",fb);
			write(writefd,file_bytes,strlen(file_bytes));
		}
		exit(0);
	}
	}


}
