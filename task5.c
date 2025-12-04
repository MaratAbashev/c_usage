#include <stdio.h>
#include <dirent.h>
#include <string.h>

void print_dir(const char *path) {
    DIR *dir;
    struct dirent *entry;

    printf("\nDirectory content: %s\n", path);

    dir = opendir(path);
    if (!dir) {
        perror("Error opening directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        printf("%s\n", entry->d_name);
    }

    if (closedir(dir) == -1) {
        perror("Error closing directory");
    }
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Directory argument is empty\n");
        return 1;
    }

    print_dir(argv[1]);
    print_dir(".");

    return 0;
}
