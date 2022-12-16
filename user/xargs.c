#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define NARGS (16)
#define ARG_SIZE (16)

/*
 * Splits a string from fd into an array of strings dest by the delimiter sep.
 * Returns number of the substrings.
 */
int strsplit(char *dest[], int fd, char sep) {
    int count = 0;
    for (char *p = dest[0], buf[2];;) {
        int ret = read(fd, buf, 1);
        if (ret != 1 || buf[0] == sep) {
            p = dest[++count];
            if (ret != 1)
                return count;
        } else
            *p++ = buf[0];
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(2, "Usage: xargs <command> [arguments]\n");
        exit(1);
    }

    char tokens[NARGS][ARG_SIZE + 1];
    char *ptokens[NARGS];

    for (int i = 0; i < NARGS; ++i)
        ptokens[i] = tokens[i];
    int len = strsplit(ptokens, 0, '\n');

    char *args[MAXARG + 1] = {0};
    memcpy(args, argv + 1, sizeof(char *) * (argc - 1));
    for (int i = 0; i < len; ++i) {
        args[argc - 1] = ptokens[i];
        int pid = fork();
        if (pid > 0)
            wait((int *)0);
        else if (pid == 0) {
            exec(argv[1], args);
            exit(0);
        } else {
            fprintf(2, "xargs: failed to create child process\n");
            exit(1);
        }
    }

    exit(0);
}
