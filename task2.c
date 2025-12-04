#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Filename is missing\n");
        return 1;
    }

    FILE* file = fopen(argv[1], "w");
    if (!file) {
        perror("Can't open file");
        return 1;
    }

    int key;
    while ((key = fgetc(stdin)) != EOF && (key != 6)) // 6 is ctrl+f
    {
        if (fputc(key, file) == EOF) {
            perror("Error writing file");
            fclose(file);
            return 1;
        }
    }
    if(fclose(file) == EOF) {
        perror("Error while closing file");
        return 1;
    }

    printf("Data added to %s\n", argv[1]);
    return 0;
}