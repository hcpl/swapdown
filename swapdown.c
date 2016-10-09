#include <unistd.h>
#include <stdio.h>

int main() {
    char *argv[] = { "/bin/sh", "-c", "/sbin/swapoff -a; /sbin/swapon -a", 0 };
    execve(argv[0], &argv[0], NULL);
    perror("execve() has failed");

    return 1;
}
