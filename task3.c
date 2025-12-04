#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Some arguments are missing\n");
        return 1;
    }
    FILE* file = fopen(argv[1], "r");
    if (!file) {
        perror("Can't open file");
        return 1;
    }
    char* endptr;
    long n = strtol(argv[2], &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "N must be number\n");
        return 1;
    }
    if (n < 0) {
        fprintf(stderr, "N is less than 0\n");
        return 1;
    }
    int key;
    do {
        if (n == 0) {
            while ((key = fgetc(file)) != EOF) {
                fputc(key, stdout);
            }
        }
        else {
            for (long i = n; i > 0; i--) {
                while ((key = fgetc(file)) != EOF) {
                    fputc(key, stdout);
                    if (key == 10) {
                        break;
                    }

                }  
            }
        }
    }
    while(key != EOF && getc(stdin) == 10);
    printf("\n");
    if(fclose(file) == EOF) {
        perror("Error while closing file");
        return 1;
    }
    return 0;
}