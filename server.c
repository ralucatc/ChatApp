/* TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <netdb.h>
char username[20][5];
int threadNumber,sockets[5];
pthread_t thread[5];
struct sockaddr_in adrese[5];
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


void *client (void *fd){
	int *sockfd = (int *)fd;
	char buff[255],msg[255];
	char usr[20];
	recvfrom(*sockfd, usr, 20, 0, NULL, NULL);
	while(1){
		if(recvfrom(*sockfd, buff, 255, 0, NULL, NULL) <= 0)
	break;
    else{
	strcpy(msg,usr);
	strcat(msg, ": ");
	strcat(msg, buff);
	fputs(msg, stdout);
	for(int i=0;i<threadNumber;i++){
		sendto(sockets[i],msg,255,0,(struct sockaddr *)&adrese[i],sizeof(adrese[i]));
		
	}
	}
	}
	
	for(int i=0;i<threadNumber;i++){
		if(*sockfd == sockets[i]){
			for(int j=i+1;j<threadNumber;j++){
				sockets[j-1]=sockets[j];
				adrese[j-1]=adrese[j];
				thread[j-1]=thread[j];
			}
				
		}
	}
	threadNumber--;
	close(*sockfd);
	free(sockfd);
	pthread_mutex_lock(&mutex);
    printf("Un client s-a deconectat!\n");
    pthread_mutex_unlock(&mutex);

}



void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, *newsockfd, portno;
     socklen_t clilen;
     char buffer[255];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
	 pthread_attr_t attr;

	  pthread_attr_init(&attr);
	  pthread_attr_setdetachstate(&attr, 1);
	  pthread_mutex_init(&mutex, NULL);
	  
	 portno = atoi(argv[1]);
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     
	 
	 set_addr(&serv_addr,NULL,INADDR_ANY,portno);
	 
	 
	 
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     while(1)
     {
		 
	  newsockfd = (int *)malloc(sizeof(int));
     *newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
			if (newsockfd < 0) 
          error("ERROR on accept");
	  
          if(pthread_create(&thread[threadNumber], &attr, client, (void *)newsockfd) != 0){
      printf("Eroare la crearea unui fir nou de executie!\n");
      exit(1);
    }
	adrese[threadNumber]=cli_addr;
	sockets[threadNumber]=*newsockfd;
	pthread_mutex_lock(&mutex);
	threadNumber++;
	printf("%d conexiuni!\n", threadNumber);
	pthread_mutex_unlock(&mutex);
	
     }
     close(*newsockfd);
     close(sockfd);
     return 0; 
}
