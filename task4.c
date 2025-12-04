#include <stdio.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Some arguments are missing\n");
        return 1;
    }

    FILE* file1 = fopen(argv[1], "r");
    if (!file1) {
        perror("Can't open file");
        return 1;
    }

    FILE* file2 = fopen(argv[2], "w");
    if (!file2) {
        perror("Can't open file2");
        fclose(file1);
        return 1;
    }

    struct stat statistic;

    if (stat(argv[1], &statistic) == -1) {
        perror("Cant get stat from file1");
        fclose(file1);
        fclose(file2);
        return 1;
    }
    
    int key;
    
    while ((key = fgetc(file1)) != EOF) {
        if (fputc(key, file2) == EOF) {
            perror("Error writing to file2");
            fclose(file1);
            fclose(file2);
            return 1;
        }
    }

    if (chmod(argv[2], statistic.st_mode) == -1) {
        perror("Cant give rights to file2");
        fclose(file1);
        fclose(file2);
        return 1;
    }

    if(fclose(file1) == EOF) {
        perror("Error while closing file1");
        fclose(file2);
        return 1;
    }

    if(fclose(file2) == EOF) {
        perror("Error while closing file2");
        return 1;
    }

    printf("File %s copied to %s\n", argv[1], argv[2]);
    return 0;
}