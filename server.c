/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

char* buffer[256]; //buffer global, pode dar erro de semaforo (um atropela o outro)

void error(char*);

int app_socket();

void app_bind(int, int);

int app_accept(int);

char* app_read(int,char*);

char* app_write(int,char*);

void * loop_app_read_write(int*);

int app_finish(char*);

int main(int argc, char *argv[])
{
    int sockfd, newsockfd1, newsockfd2;

    pthread_t T1, T2;

    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    sockfd = app_socket();

    app_bind(sockfd, atoi(argv[1]));

    listen(sockfd,5);

    newsockfd1 = app_accept(sockfd);
    newsockfd2 = app_accept(sockfd);
//************************ Jogar numa função para cada conexão entre clientes (ser uma thread)
    //void* conexão_CC(newsockfd1,newsockfd2){

    int socketes1[2] = {newsockfd1,newsockfd2}; //jogo de links para conexão C-S-C
    int socketes2[2] = {newsockfd2,newsockfd1};

    pthread_create(&T1,NULL,loop_app_read_write,&socketes1); //le oque o C1 diz e escreve no C2

    pthread_create(&T2,NULL,loop_app_read_write,&socketes2); // same as above, but C2 to C1
    //}
//************************
    pthread_join(T1,NULL);
    pthread_join(T2,NULL);
    

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

void app_bind(int sockfd, int portno) {
    struct sockaddr_in serv_addr;

    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }
}

int app_accept(int sockfd) {
    int clilen;
    int newsockfd;
    struct sockaddr_in cli_addr;

    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr, (unsigned int *) &clilen);
    
    if (newsockfd < 0) {
        error("ERROR on accept");
    }

    return newsockfd;
}

void * loop_app_read_write(int* socketes){
    char * check;

    do{
        check = app_read(socketes[0],buffer);
        check = app_write(socketes[1],buffer);
    } while (!app_finish(check));
    
    return 0;
}



char* app_write(int sockfd, char* buffer) {
    int n;

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

    printf("Data Info: %s\n", buffer);
    return buffer;
}