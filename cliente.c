#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string.h>
#include <unistd.h>

int sockfd; //variavel global pq sim.

void * loop_app_write(char *);
void * loop_app_read(char *);

void error(char *);

int app_finish(char *);

int app_socket();

void app_connect(int, int, struct hostent *);

char* app_write(int, char*);

char* app_read(int, char*);


int main(int argc, char *argv[]) {
    struct hostent *server;

    char buffer[256];
    bzero(buffer,256);  //criei um buffer e limpei ele

    pthread_t Tread,Twrite;

    if (argc < 3) {
       fprintf(stderr, "usage %s hostname port\n", argv[0]);
       exit(0);
    }

    sockfd = app_socket();
    server = gethostbyname(argv[1]);

    
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
 
    app_connect(sockfd, atoi(argv[2]), server);  //aqui dentro ele estabelece a conexão

    pthread_create(&Twrite,NULL,loop_app_write,&buffer);

    pthread_create(&Tread,NULL,loop_app_read,&buffer);

    pthread_join(Twrite,NULL);
    pthread_join(Tread,NULL);
    
    return 0;
}

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int app_finish(char *buffer) {
    if (strlen(buffer)) {
        char *token;

        token = strtok(buffer, "\n\r");

        return !strcmp(token, "bye");
    }

    return 0;
}

int app_socket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    return sockfd;
}

void app_connect(int sockfd, int portno, struct hostent *server) {
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
    }

}

void * loop_app_write(char *buffer){

    char* check;

    do{
        check = app_write(sockfd,buffer); //sempre irá pedir para dar write
    } while (!app_finish(check)); //sempre ira verificar se todo write é um fim

    return 0;
}


void * loop_app_read(char *buffer){

    char* check;

    do{
        check = app_read(sockfd,buffer); //sempre disponivel para receber mensagens
    } while (!app_finish(check)); // sempre irá ver se o outro cliente deu um fim
    
    return 0;
}


char* app_write(int sockfd, char* buffer) {
    int n;

    //printf("Escreva a sua mensagem:");

    bzero(buffer, 256);
    fgets(buffer, 255, stdin);
    
    printf("\n");
    n = write(sockfd, buffer, strlen(buffer));

    if (n < 0) {
        error("ERROR writing to socket");
    }
    return buffer;
}

char* app_read(int sockfd, char* buffer) {
    int n;

    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);

    if (n < 0) {
        error("ERROR reading from socket");
    }

    printf("Resposta: %s\n", buffer);
    return buffer;
}

