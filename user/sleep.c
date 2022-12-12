#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(2, "Usage: sleep <ticks>\n");
        exit(1);
    }

    // `atoi` tries to accumulate digits from the first character until it meets
    // an non-digit character. The default accumulated sum (`n` in its function
    // body) is 0, so that `atoi` can regarded safe even if it receives an
    // illegal string.
    
    int pid = fork();
    if (pid > 0) {
        // in the parent process
        wait((int *) 0); // wait for the child process to exit
        exit(0);
    } else if (pid == 0) {
        // in the child process
        sleep(atoi(argv[1]));
        exit(0);
    } else {
        // fork error
        fprintf(2, "sleep %s: error when creating a child process\n", argv[1]);
        exit(1);
    }
}
