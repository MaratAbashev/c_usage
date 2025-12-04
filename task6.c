#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>

struct dir_stats {
    char   largest_name[1024];
    off_t  largest_size;
    off_t  total_size;
    int    file_count;
};

/* base_dir  – путь к базовому каталогу (аргумент 1)
 * sub_name  – имя подкаталога относительно base_dir
 *            для текущего каталога используем "."
 * label     – как выводить это имя в отчёте (".", "audio", и т.п.)
 */
int process_subdir(const char *base_dir,
                   const char *sub_name,
                   const char *label,
                   FILE *out)
{
    char path[2048];
    DIR *dir;
    struct dirent *entry;
    struct dir_stats st = { "", 0, 0, 0 };

    snprintf(path, sizeof(path), "%s/%s", base_dir, sub_name);

    dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "Error opening dir '%s': %s\n",
                path, strerror(errno));
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char file_path[2048];
        struct stat sb;

        snprintf(file_path, sizeof(file_path), "%s/%s", path, entry->d_name);

        if (stat(file_path, &sb) == -1) {
            fprintf(stderr, "Error stat '%s': %s\n",
                    file_path, strerror(errno));
            continue;
        }

        if (S_ISREG(sb.st_mode)) {
            st.file_count++;
            st.total_size += sb.st_size;

            if (sb.st_size > st.largest_size) {
                st.largest_size = sb.st_size;
                strncpy(st.largest_name, entry->d_name,
                        sizeof(st.largest_name) - 1);
                st.largest_name[sizeof(st.largest_name) - 1] = '\0';
            }
        }
    }

    if (closedir(dir) == -1) {
        fprintf(stderr, "Error closing dir '%s': %s\n",
                path, strerror(errno));
    }

    fprintf(out, "%s\t%d\t%lld\t%s\n",
            label,
            st.file_count,
            (long long)st.total_size,
            st.file_count > 0 ? st.largest_name : "-");

    printf("%s\t%d\t%lld\t%s\n",
           label,
           st.file_count,
           (long long)st.total_size,
           st.file_count > 0 ? st.largest_name : "-");

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <dir> <output_file>\n", argv[0]);
        return 1;
    }

    const char *base_dir = argv[1];
    const char *out_name = argv[2];

    DIR *dir = opendir(base_dir);
    if (!dir) {
        perror("Error opening base directory");
        return 1;
    }

    FILE *out = fopen(out_name, "w");
    if (!out) {
        perror("Error opening output file");
        closedir(dir);
        return 1;
    }

    struct dirent *entry;

    fprintf(out, "subdir\tfiles\tbytes\tlargest_file\n");
    printf("subdir\tfiles\tbytes\tlargest_file\n");

    /* 1. сначала обрабатываем сам base_dir как текущий каталог */
    process_subdir(base_dir, ".", base_dir, out);

    /* 2. затем все его подкаталоги, как раньше */
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char path[2048];
        struct stat sb;

        snprintf(path, sizeof(path), "%s/%s", base_dir, entry->d_name);

        if (stat(path, &sb) == -1) {
            fprintf(stderr, "Error stat '%s': %s\n",
                    path, strerror(errno));
            continue;
        }

        if (S_ISDIR(sb.st_mode)) {
            process_subdir(base_dir, entry->d_name, entry->d_name, out);
        }
    }

    if (closedir(dir) == -1) {
        perror("Error closing base directory");
    }

    if (fclose(out) == EOF) {
        perror("Error closing output file");
        return 1;
    }

    return 0;
}
