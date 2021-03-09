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
#include <math.h>

char username[20],password[20],buffer[30]=" ";
unsigned char epassword[20],buffer2[30]=" ";
pthread_mutex_t mutex;


int set_addr(struct sockaddr_in *addr, char *name, u_int32_t inaddr, short sin_port){ //pentru completarea structurii sockadrr_in
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

void encrypt(unsigned char *in, unsigned char *out, int len){
    unsigned char prev = 0;
    for(int i=0;i<len;i++){
        out[i]=(((in[i]+10)+3))-prev;
        prev=out[i];
    }
}

void login_user(){
    int ok=0;
    FILE *f;
    if((f=fopen("users.txt","r"))==NULL){
		error("Error opening file");
     exit(1);
	}
    printf("\nUsername: ");
    scanf("%s", username);
    printf("Password: ");
    scanf("%s", password);
    encrypt(password,epassword,strlen(password));
    
    while(!feof(f)){
        fscanf(f,"%s",buffer);
        fscanf(f,"%s",buffer2);
        if((strcmp(username,buffer)==0)&&(strcmp(epassword,buffer2)==0))
            ok=1;
    }
    
    if(ok==1)
        printf("Succesful login\n");
    else{
        printf("Please enter the correct username and password");
        login_user();
    }
    
    fclose(f);
}

void register_user(){
    FILE *users;
    
	if((users=fopen("users.txt", "a"))==NULL){
     error("Error opening file");
     exit(1);
    }
    
    printf("Enter Username: "); 
    scanf("%s", username);
    printf("Enter password: "); 
    scanf("%s", password);
    encrypt(password,epassword,strlen(password));
    fprintf(users,"%s",username);
    fprintf(users,"\n");
    fprintf(users,"%s",epassword);
    fprintf(users,"\n");
    fprintf(users,"\n");
    
    fclose(users);
    
    printf("\nNow login with the username and password\n");
    login_user();
}

int main(int argc, char *argv[])
{
    int *sockfd=(int *)malloc(sizeof(int)), portno;//adresa socket si numarul portului
    struct sockaddr_in serv_addr,remote_addr;//adresa serverului si adresa clientului
    char buffer[256];
	
	
	pthread_t read_thr,write_thr; //cate un thread pentru citire si scriere
	
	pthread_attr_t attr;	
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, 1);
	pthread_mutex_init(&mutex, NULL);
	
	
	
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(1);
    }
	
	
    portno = atoi(argv[2]);
		
		
		
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);//deschidem un socket
	
    if (sockfd < 0) {
        error("ERROR opening socket");
	}
	
    set_addr(&serv_addr, NULL, INADDR_ANY, 0);//completam structura serv_addr
	
	if(bind(*sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1){//asociem soclului o adresa ip si portul
		error("eroare la bind()!\n");
		exit(1);
	}
	
    if(set_addr(&remote_addr,argv[1],0,portno) == -1){// completeaza structura remote_addr (a clientului) pentru adresa lui ip si socket-ul dat ca param
		error("Eroare la adresa ! \n");
		exit(1);
	}
	
	
    if (connect(*sockfd,(struct sockaddr *) &remote_addr,sizeof(remote_addr)) < 0) //face legatura dintre descriptorul de socket si adresa user-ului
        error("ERROR connecting");
    
  
    
    int c;
    
    printf("Already have an username and password?\n (Yes - 1 No - 2): \n");
	while(c!=2 && c!=1){
    scanf("%d", &c);
	printf("1 or 2\n");
	}
	if(c==2){
        register_user();
    }
    else if(c==1){
        login_user();
    }
    
    
	
	if(pthread_create(&read_thr, &attr , getMsg, (void *)sockfd) != 0){ //thread pentru primire mesaj
		error("Eroare fir de executie primire mesaj\n");
		exit(1);
	}
	
	if(pthread_create(&write_thr, &attr , sendMsg, (void *)sockfd) != 0){ //thread pentru trimitere mesaj
		error("Eroare fir de executie trimitere mesaj\n");
		exit(1);
	}
	
    while(1){}
    
    return 0;
}
