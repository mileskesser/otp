#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

// Function to perform decryption. You need to implement this.
void decrypt(char *ciphertext, char *key, char *plaintext, int length) {
    // Implement decryption here. This is a placeholder function.
    // Remember: Decryption is essentially the reverse of your encryption process.
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
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
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) 
          error("ERROR on accept");

        // Fork child process to handle the client's request
        int pid = fork();
        if (pid < 0) {
            error("ERROR on fork");
        }
        if (pid == 0) {  // This is the child process
            close(sockfd);
            bzero(buffer, 256);
            
            // Read ciphertext from client
            n = read(newsockfd, buffer, 255);
            if (n < 0) error("ERROR reading from socket");
            
            // Assume the key is sent immediately after the ciphertext
            char key[256];
            bzero(key, 256);
            n = read(newsockfd, key, 255);
            if (n < 0) error("ERROR reading from socket");
            
            char plaintext[256];
            decrypt(buffer, key, plaintext, n); // Perform decryption
            
            // Send plaintext back to client
            n = write(newsockfd, plaintext, strlen(plaintext));
            if (n < 0) error("ERROR writing to socket");
            
            exit(0);
        } else {
            close(newsockfd);  // Parent doesn't need this
        }
    }

    close(sockfd);
    return 0; 
}
