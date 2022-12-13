#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define MAX_SEARCHED_NUM (35) /* Maximum of the searched numbers. */

/*
 * Performs a prime sieve from a file.
 *
 * It reads characters as the filtered numbers from fd, does its own filtering
 * process, then writes the new stream of characters to its child process.
 *
 * Limited to its implementation, the maximum of numbers parsed from fd should
 * not more than 255, otherwise there will be an overflow.
 */
int sieve(int fd) {
    int p[2];       /* File descriptors for pipe(). */
    int pid;        /* PID for fork(). */
    char buf[2];    /* Buffer for a single character. */
    char first_num; /* The first character read from pipe. */

    /* Try to read the first character, for it should be printed out. */
    if (read(fd, buf, 1) != 1) {
        /* Meet EOF. All the numbers are searched. */

        /*
         * Maybe these close() operations can be omitted here, for the file
         * descriptors are released when exit().
         */
        close(fd);
        close(p[0]);
        close(p[1]);
        exit(0);
    }
    first_num = buf[0];
    fprintf(1, "prime %d\n", buf[0]);

    /*
     * This conditional structure helps debugging the limitation of the number
     * of file descriptors. Previously, the program failed to write bytes to
     * files after some processes. Determining whether pipe() executes
     * successfully is helpful here.
     *
     * The concrete limit has not been found after a simple web search. From the
     * result of the previous "experiments", it may be 16. A small number of
     * open file descriptors forces some files to be closed quickly. See the
     * comments below.
     */
    if (pipe(p) != 0) {
        fprintf(2, "primes: sieve(%d): failed to create pipe\n", getpid());
    }
    pid = fork();

    /*
     * The parent process parses numbers from the characters and writes to
     * the child process.
     */
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
        /*
         * Try to read other possible characters and pipe them.
         *
         * NOTE there is an assumption that the maximum searched numbers is less
         * than 256 so that a char can hold the value without overflow. This
         * might be regarded as a trick because the lab only requires all the
         * prime numbers no more than 35. A more robust way is to pipe literal
         * representations of integers, and then to parse them back integers.
         */
        while (read(fd, buf, 1) == 1) {
            if (buf[0] % first_num != 0) {
                write(p[1], buf, 1);
            }
        }
        close(fd);
        close(p[0]);
        close(p[1]);

        /*
         * It is not very useful to output a result, especially when the
         * standard output redirects to a file. Removing the line causes the
         * outputs of this process and the shell process intermixed (i.e. a "$"
         * from the shell appearing among the output of this program). However,
         * it slightly violates the example output of the lab instructions.
         */
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
    /* Write the initial character stream to pipe. */
    for (char i = 2; i <= MAX_SEARCHED_NUM; ++i) {
        buf[0] = i;
        write(p[1], buf, 1);
    }
    close(p[1]);

    sieve(p[0]);
    close(p[0]);

    exit(0);
}
