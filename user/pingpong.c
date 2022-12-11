#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void closeall(int *p, int offset) {
    int i;
    for (i = offset; i < 4; ++i) {
        close(p[i]);
    }
}

int main(int argc, char *argv[]) {
    int p[4];
    int childStatus;
    char buf[1];

    pipe(p);
    pipe(p + 2);
    int pid = fork();
    if (pid == 0) {
        if (read(p[0], buf, 1) != 1) {
            fprintf(2, "pingpong: child process has not been received byte from parent process\n");
            closeall(p, 1);
            exit(1);
        }
        close(p[1]);
        fprintf(1, "%d: received ping\n", getpid());
        if (write(p[3], "b", 1) != 1) {
            fprintf(2, "pingpong: child process has not sent byte to parent process\n");
            closeall(p, 2);
            exit(1);
        }
        close(p[3]);
        exit(0);
    } else if (pid > 0) {
        if (write(p[1], "a", 1) != 1) {
            fprintf(2, "pingpong: parent process has not sent byte to child process\n");
            closeall(p, 0);
            exit(1);
        }
        wait(&childStatus);
        close(p[0]);
        if (childStatus != 0) {
            fprintf(2, "pingpong: parent process has detected error in child process\n");
            exit(1);
        }
        if (read(p[2], buf, 1) != 1) {
            fprintf(2, "pingpong: parent process has not been received byte from child process\n");
            close(p[2]);
            exit(1);
        }
        close(p[2]);
        fprintf(1, "%d: received pong\n", getpid());
        exit(0);
    } else {
        fprintf(2, "pingpong: error when creating a child process\n");
        closeall(p, 0);
        exit(1);
    }
}
