
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void setupAddressStruct(struct sockaddr_in* address, int portNumber){
    memset(address, '\0', sizeof(*address)); 
    address->sin_family = AF_INET;
    address->sin_port = htons(portNumber);
    address->sin_addr.s_addr = INADDR_ANY;
}

void otp_encrypt_decrypt(char *text, char *key, char *result, int length, int encrypt) {
    for (int i = 0; i < length; ++i) {
        int textVal = (text[i] == ' ') ? 26 : text[i] - 'A';
        int keyVal = (key[i] == ' ') ? 26 : key[i] - 'A';
        int resultVal;

        if (encrypt) {
            resultVal = (textVal + keyVal) % 27;
        } else {
            resultVal = (textVal - keyVal + 27) % 27;
        }

        result[i] = (resultVal == 26) ? ' ' : 'A' + resultVal;
    }
    result[length] = '\0';
}

void trimNewline(char *str) {
    int len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

void handleConnection(int connectionSocket) {
    char buffer[1024];
    memset(buffer, '\0', 1024);
    int charsRead = recv(connectionSocket, buffer, 1023, 0); 
    if (charsRead < 0) {
        error("ERROR reading from socket");
    }

    char *separator = strstr(buffer, "|||");
    if (separator == NULL) {
        error("ERROR: separator not found");
    }
    *separator = '\0';
    char *plaintext = buffer;
    char *key = separator + 3;

   trimNewline(plaintext);
    trimNewline(key);

    char encryptedText[1024];
    otp_encrypt_decrypt(plaintext, key, encryptedText, strlen(plaintext), 1);

    charsRead = send(connectionSocket, encryptedText, strlen(encryptedText), 0); 
    if (charsRead < 0) {
        error("ERROR writing to socket");
    }

    close(connectionSocket);
}

int main(int argc, char *argv[]){
    int listenSocket, connectionSocket;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);

    if (argc < 2) { 
        fprintf(stderr,"USAGE: %s port\n", argv[0]); 
        exit(1);
    }

    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) error("ERROR opening socket");

    setupAddressStruct(&serverAddress, atoi(argv[1]));

    if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) error("ERROR on binding");

    listen(listenSocket, 5); 

    while(1){
        connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); 
        if (connectionSocket < 0) error("ERROR on accept");

        int pid = fork();
        if (pid < 0) {
            error("ERROR on fork");
        } else if (pid == 0) {
            // child 
            close(listenSocket);
            handleConnection(connectionSocket);
            exit(0);
        } else {
            // parent
            close(connectionSocket);
            while(waitpid(-1, NULL, WNOHANG) > 0);
        }
    }

    close(listenSocket); 
    return 0;
}

