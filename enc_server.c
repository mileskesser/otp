#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#define MAX_CONNECTIONS 5

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void custom_encrypt(char *plaintext, char *key, char *ciphertext) {
    int i, p, k;
    for (i = 0; plaintext[i] != '\0' && key[i] != '\0'; i++) {
        p = (plaintext[i] == ' ') ? 26 : plaintext[i] - 'A';
        k = (key[i] == ' ') ? 26 : key[i] - 'A';
        int mod = (p + k) % 27;
        ciphertext[i] = (mod == 26) ? ' ' : mod + 'A';
    }
    ciphertext[i] = '\0';
}

void handle_connection(int client_socket) {
    char buffer[256];
    char key[256];
    char ciphertext[256];

    read(client_socket, buffer, 255);
    read(client_socket, key, 255);
    custom_encrypt(buffer, key, ciphertext);

    write(client_socket, ciphertext, strlen(ciphertext));
    close(client_socket);
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");

    listen(sockfd, MAX_CONNECTIONS);
    clilen = sizeof(cli_addr);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) 
            error("ERROR on accept");

        int pid = fork();
        if (pid < 0)
            error("ERROR on fork");
        if (pid == 0)  {
            close(sockfd);
            handle_connection(newsockfd);
            exit(0);
        }
        else close(newsockfd);
    }

    close(sockfd);
    return 0; 
}
