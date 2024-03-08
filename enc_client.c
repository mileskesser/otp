#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg, int exitCode) {
    fprintf(stderr, "%s\n", msg); 
    exit(exitCode);
}

void setupAddressStruct(struct sockaddr_in* address, int portNumber) {
    memset(address, '\0', sizeof(*address));
    address->sin_family = AF_INET;
    address->sin_port = htons(portNumber);
    struct hostent* serverHostInfo = gethostbyname("localhost");
    if (serverHostInfo == NULL) {
        error("CLIENT: ERROR, no such host", 2);
    }
    memcpy((char*)&address->sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);
}

int readFileContents(const char *filename, char **buffer) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Could not open file %s for reading\n", filename);
        return -1;
    }

    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);

    *buffer = malloc(length + 1);
    if (*buffer) {
        fread(*buffer, 1, length, f);
        (*buffer)[length] = '\0'; 
    }
    fclose(f);
    return length;
}

int main(int argc, char *argv[]) {
    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    char *plaintextBuffer, *keyBuffer;
    char response[256]; 

    if (argc < 4) {
        error("USAGE: enc_client plaintext key port", 1);
    }

    int plaintextLength = readFileContents(argv[1], &plaintextBuffer);
    if (plaintextLength < 0) {
        error("Error reading plaintext file", 1);
    }

    int keyLength = readFileContents(argv[2], &keyBuffer);
    if (keyLength < 0) {
        free(plaintextBuffer);
        error("Error reading key file", 1);
    }

    if (keyLength < plaintextLength) {
        free(plaintextBuffer);
        free(keyBuffer);
        error("Error: key is too short", 1);
    }



    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        error("CLIENT: ERROR opening socket", 2);
    }

    setupAddressStruct(&serverAddress, atoi(argv[3]));

    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        error("CLIENT: ERROR connecting", 2);
    }

    char *message = malloc(plaintextLength + keyLength + 4);
    if (message == NULL) {
        free(plaintextBuffer);
        free(keyBuffer);
        error("Error allocating memory for message", 1);
    }
    sprintf(message, "%s|||%s", plaintextBuffer, keyBuffer);

    charsWritten = send(socketFD, message, strlen(message), 0);
    if (charsWritten < 0) error("CLIENT: ERROR writing to socket", 2);
    if (charsWritten < strlen(message)) printf("CLIENT: WARNING: Not all data written to socket!\n");

    free(plaintextBuffer);
    free(keyBuffer);
    free(message);

    memset(response, '\0', sizeof(response));
    charsRead = recv(socketFD, response, sizeof(response) - 1, 0);
    if (charsRead < 0) error("CLIENT: ERROR reading from socket", 2);

    printf("%s\n", response); 

    close(socketFD);
    return 0; 
}
