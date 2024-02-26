#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

//

void error(const char *msg) {
    perror(msg);
    exit(0);
}

void readFileAndSend(int sockfd, const char* filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Could not open file %s\n", filename);
        exit(1);
    }

    // Find out the size of the file
    fseek(file, 0L, SEEK_END);
    long bufsize = ftell(file);
    fseek(file, 0L, SEEK_SET);

    // Allocate memory for the entire content
    char *buffer = malloc(sizeof(char) * (bufsize + 1));

    // Read the file into memory
    size_t newLen = fread(buffer, sizeof(char), bufsize, file);
    if (ferror(file) != 0) {
        fputs("Error reading file", stderr);
    } else {
        buffer[newLen++] = '\0'; // Just to be safe.
    }

    fclose(file);

    // Send file content to server
    int n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) {
        error("ERROR writing to socket");
    }

    free(buffer);
}

int main(int argc, char *argv[]) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 4) {
       fprintf(stderr,"usage %s ciphertext key port\n", argv[0]);
       exit(0);
    }

    portno = atoi(argv[3]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    // Send ciphertext and key to server
    readFileAndSend(sockfd, argv[1]); // Send ciphertext
    readFileAndSend(sockfd, argv[2]); // Send key

    // Receive decrypted plaintext from server
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer);

    close(sockfd);
    return 0;

}
