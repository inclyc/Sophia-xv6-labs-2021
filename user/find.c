#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

#define MAX_PATH_SIZE 100

char *pathbase(char *path) {
    char *p;
    static char buf[MAX_PATH_SIZE + 1];

    // Find the first character after the last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; --p)
        ;

    return strcpy(buf, p + 1);
}

char *pathjoin(char *path, char *prefix, char *dst) {
    int len = strlen(prefix);

    strcpy(dst, prefix);
    dst[len] = '/';
    strcpy(dst + len + 1, path);

    return dst;
}

int find(char *path, char *fname) {
    int fd = open(path, 0);
    if (fd < 0) {
        fprintf(2, "find: cannot open path %s\n", path);
        return 1;
    }
    struct stat st;
    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat path %s\n", path);
        return 1;
    }

    switch (st.type) {
    case T_FILE:
        if (strcmp(pathbase(path), fname) == 0) {
            printf("%s\n", path);
        }
        break;

    case T_DIR:
        if (strlen(path) + 1 + DIRSIZ + 1 > MAX_PATH_SIZE) {
            fprintf(2, "find: path %s is too long\n", path);
            break;
        }

        struct dirent de;
        while (read(fd, &de, sizeof(de)) == sizeof(de)) {
            if (de.inum == 0 || strcmp(de.name, ".") == 0 ||
                strcmp(de.name, "..") == 0) {
                continue;
            }

            char buf[MAX_PATH_SIZE + 1];
            char *p = pathjoin(de.name, path, buf);
            if (stat(p, &st) < 0) {
                fprintf(2, "find: cannot stat path with dirent name %s\n", p);
                continue;
            }

            find(p, fname);
        }
        break;
    }

    close(fd);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(2, "Usage: find <path> <filename>\n");
        exit(1);
    }

    exit(find(argv[1], argv[2]));
}
