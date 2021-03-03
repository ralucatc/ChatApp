/*
 filename server_ipaddress portno
 
 argv[0] filename
 argv[1] server_ipaddress
 argv[2] portno
 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

char username[20];
pthread_mutex_t mutex;


int set_addr(struct sockaddr_in *addr, char *name, u_int32_t inaddr, short sin_port){
  struct hostent *h;
  memset((void *) addr, 0, sizeof(*addr));
  addr->sin_family = AF_INET;
  if(name != NULL){
    h = gethostbyname(name);
    if(h == NULL)
      return -1;
    addr->sin_addr.s_addr = *(u_int32_t *) h->h_addr_list[0];
  }
  else addr->sin_addr.s_addr = htonl(inaddr);
  addr->sin_port = htons(sin_port);
  return 0;
}




void *getMsg(void *fd){
	int *sockfd = (int *)fd;
	char buff[255];
	
	while(1){
		if(recvfrom(*sockfd, buff, 255, 0, NULL, NULL) < 0){
           continue;
		}else{
			printf("%s", buff);
		}
	}
	close(*sockfd);
	free(sockfd);
	return NULL;
}



void *sendMsg(void *fd){
	int *sockfd = (int *)fd;
	char buff[255];
	send(*sockfd,username,20,0);
	while(1){
		fgets(buff,255,stdin);
		fputs("\033[A\033[2K",stdout);
		send(*sockfd,buff,255,0);
	}
	close(*sockfd);
	free(sockfd);
	return NULL;
}





void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int *sockfd=(int *)malloc(sizeof(int)), portno, n;
    struct sockaddr_in serv_addr,remote_addr;
    char buffer[256];
	
	
	pthread_t read_thr,write_thr;
	pthread_attr_t attr;
	
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, 1);
	pthread_mutex_init(&mutex, NULL);
	
	
	
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(1);
    }
	
	
    portno = atoi(argv[2]);
		
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
    if (sockfd < 0) {
        error("ERROR opening socket");
	}
	
    set_addr(&serv_addr, NULL, INADDR_ANY, 0);
	
	if(bind(*sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1){
		printf("eroare la bind()!\n");
		exit(1);
	}
	
    if(set_addr(&remote_addr,argv[1],0,portno) == -1){
		printf("Eroare la adresa ! \n");
		exit(1);
	}
	
	
    if (connect(*sockfd,(struct sockaddr *) &remote_addr,sizeof(remote_addr)) < 0) 
        error("ERROR connecting");
    printf("Introduceti un nickname :  ");
	scanf("%s",username);
	
	
	if(pthread_create(&read_thr, &attr , getMsg, (void *)sockfd) != 0){
		printf("Eroare fir de executie primire mesaj\n");
		exit(1);
	}
	
	if(pthread_create(&write_thr, &attr , sendMsg, (void *)sockfd) != 0){
		printf("Eroare fir de executie trimitere mesaj\n");
		exit(1);
	}
	
	
	
    while(1)
    {
     
    }
    return 0;
}
