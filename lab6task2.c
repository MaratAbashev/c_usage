#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <limits.h>

static int file_exists(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0);
}

static long long copy_file_bytes(const char *src, const char *dst, mode_t mode) {
    int in = open(src, O_RDONLY);
    if (in < 0) return -1;

    int out = open(dst, O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (out < 0) { close(in); return -1; }

    char buf[64 * 1024];
    long long total = 0;

    for (;;) {
        ssize_t r = read(in, buf, sizeof(buf));
        if (r < 0) { total = -1; break; }
        if (r == 0) break;

        ssize_t off = 0;
        while (off < r) {
            ssize_t w = write(out, buf + off, (size_t)(r - off));
            if (w < 0) { total = -1; break; }
            off += w;
        }
        if (total < 0) break;
        total += r;
    }

    if (close(in) < 0 && total >= 0) total = -1;
    if (close(out) < 0 && total >= 0) total = -1;

    if (total >= 0) {
        if (chmod(dst, mode & 07777) != 0) return -1;
    }
    return total;
}

static void reap_one(int *running) {
    int status = 0;
    pid_t p = waitpid(-1, &status, 0);
    if (p > 0) (*running)--;
}

static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s DIR1 DIR2 N\n", prog);
    fprintf(stderr, "  DIR1 - source directory\n");
    fprintf(stderr, "  DIR2 - destination directory\n");
    fprintf(stderr, "  N    - max parallel copy processes (N > 0)\n");
}

int main(int argc, char **argv) {
    if (argc != 4) {
        usage(argv[0]);
        return 1;
    }

    const char *dir1 = argv[1];
    const char *dir2 = argv[2];

    char *end = NULL;
    long nlong = strtol(argv[3], &end, 10);
    if (!end || *end != '\0' || nlong <= 0 || nlong > 100000) {
        fprintf(stderr, "Invalid N: %s (must be positive integer)\n", argv[3]);
        return 1;
    }
    int N = (int)nlong;

    DIR *d = opendir(dir1);
    if (!d) { perror("opendir Dir1"); return 1; }

    int running = 0;
    struct dirent *de;

    while ((de = readdir(d)) != NULL) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            continue;

        char src[PATH_MAX], dst[PATH_MAX];
        snprintf(src, sizeof(src), "%s/%s", dir1, de->d_name);
        snprintf(dst, sizeof(dst), "%s/%s", dir2, de->d_name);

        struct stat st;
        if (stat(src, &st) != 0) continue;
        if (!S_ISREG(st.st_mode)) continue;

        if (file_exists(dst)) continue;

        while (running >= N) {
            reap_one(&running);
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            break;
        }

        if (pid == 0) {
            long long bytes = copy_file_bytes(src, dst, st.st_mode);
            if (bytes < 0) {
                fprintf(stderr, "PID=%ld Failed to copy %s -> %s: %s\n",
                        (long)getpid(), de->d_name, dst, strerror(errno));
                _exit(2);
            }

            printf("PID=%ld Copied file=%s bytes=%lld\n",
                   (long)getpid(), de->d_name, bytes);
            _exit(0);
        } else {
            running++;
        }
    }

    closedir(d);

    while (running > 0) {
        reap_one(&running);
    }

    return 0;
}
