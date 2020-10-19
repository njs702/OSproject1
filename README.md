# OSproject1

Named pipe code description for communication between processes

### 1. Header file & Macro & Function
```
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
```
mkfifo를 통해 named pipe 생성을 하기 위한 pathname FIFO1, FIFO2 선언. 리눅스 시스템에서 fork() 및 프로세스 통신을 위한 헤더파일들을 추가했다.

### 2. main function
```
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
```
mkfifo를 사용해 named pipe를 사용하기 위한 path를 생성한다. mkfifo 함수는 성공시 0을 리턴하고 실패시 -1을 리턴하기 때문에 path를 생성하지 못했는데 !EEXIST(이미 프로세스가 존재하지 않는 경우) 처음 path 생성 여부를 판단한다.

Child를 fork해서 0인 경우(child인 경우), 각각의 readfd와 writefd를 선언해주고 server와 통신을 하기 위해 server function을 사용한다. 자식이 아닌 경우 반대로 readfd와 writefd를 선언해주고 client와 통신을 하기 위해 client function을 사용한다.

waitpid(childpid,NULL,0)을 통해 child가 terminate 될 때까지 부모 프로세스가 기다리게 된다.

### 3. server function
```
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
```
### 4. client function
```
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
```
