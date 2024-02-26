#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void generateKey(int length) {
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    const int charsetSize = sizeof(charset) - 1;
    
    srand(time(NULL)); // Seed the random number generator

    for(int i = 0; i < length; i++) {
        printf("%c", charset[rand() % charsetSize]);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        fprintf(stderr, "Usage: %s keylength\n", argv[0]);
        return 1;
    }

    int length = atoi(argv[1]);
    if(length <= 0) {
        fprintf(stderr, "Key length must be a positive integer.\n");
        return 1;
    }

    generateKey(length);
    return 0;
}
