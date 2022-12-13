#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define MAX_SEARCHED_NUM (35)

/*
 * Performs a prime sieve from a file.
 *
 * It reads characters as the filtered numbers from fd, does its own filtering
 * process, then writes the new stream of characters to its child process.
 *
 * Limited to its implementation, the maximum of numbers parsed from fd should
 * be not more than 255, otherwise there will be an overflow.
 */
int sieve(int fd) {
    int p[2];
    char buf[2]; /* Buffer for a single character. */

    /* Try to read the first character, for it should be printed out. */
    if (read(fd, buf, 1) != 1) {
        /* Meet EOF. All the numbers are searched. */
        close(fd);
        close(p[0]);
        close(p[1]);
        exit(0);
    }
    char first_num = buf[0];
    printf("prime %d\n", buf[0]);

    if (pipe(p) != 0) {
        fprintf(2, "primes: sieve(%d): failed to create pipe\n", getpid());
    }

    int pid = fork();
    if (pid == 0) {
        /*
         * The child process accepts the new fd and continues to execute a new
         * sieve().
         */

        /*
         * NOTE that fd (p[0] from the parent process) should be released in
         * advance of calling sieve(). The file is useless for the child
         * process; the child only needs to form a recursion. But it is copied
         * after a fork(), and still alive even in sieve() of the child process.
         * To release the file thoroughly, it must be closed here.
         */
        close(fd);
        close(p[1]);

        sieve(p[0]);

        close(p[0]);
        exit(0);
    } else if (pid > 0) {
        while (read(fd, buf, 1) == 1) {
            if (buf[0] % first_num != 0) {
                write(p[1], buf, 1);
            }
        }
        close(fd);
        close(p[0]);
        close(p[1]);
        wait((int *)0);
        exit(0);
    } else {
        fprintf(2, "primes: sieve(%d): failed to create a child process\n",
                getpid());
        close(fd);
        close(p[0]);
        close(p[1]);
        exit(-1);
    }
}

int main(int argc, char *argv[]) {
    int p[2];
    char buf[2] = {0};

    pipe(p);
    /* Initial characters. */
    for (char i = 2; i <= MAX_SEARCHED_NUM; ++i) {
        buf[0] = i;
        write(p[1], buf, 1);
    }
    close(p[1]);

    sieve(p[0]);
    close(p[0]);

    exit(0);
}
