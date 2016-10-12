#define _POSIX_C_SOURCE 1

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static volatile pid_t child_pid = 0;

static void propagator(int sig) {
    if (child_pid > 0) {
        kill(child_pid, sig);
    }
}

#define exit_error(message) \
    perror(message); \
    exit(EXIT_FAILURE);

#define exit_error_if(cond, message) \
    if (cond) { \
        exit_error(message); \
    }

#define SBIN_PATH(executable) "/sbin/" executable

static void _call_handler(char const *executable,
                          char *executable_full_path) {
    assert(!strcmp(executable, "swapon") || !strcmp(executable, "swapoff"));

    pid_t waitpid_retval;
    int status;

    child_pid = fork();
    exit_error_if(child_pid == -1, "fork");

    if (child_pid == 0) {
        char *argv[] = { executable_full_path, "-a", NULL };
        char *envp[] = { NULL };
        execve(argv[0], argv, envp);
        exit_error(executable);
    }

    waitpid_retval = waitpid(child_pid, &status, 0);
    exit_error_if(waitpid_retval == -1, "waitpid");
    assert(waitpid_retval == child_pid);

    if (WIFSIGNALED(status)) {
        fprintf(stderr, "%s terminated by a signal %d\n",
            executable, WTERMSIG(status));
        exit(EXIT_FAILURE);
    } else if (WIFEXITED(status)) {
        int retval = WEXITSTATUS(status);

        if (retval != 0) {
            fprintf(stderr, "%s - non-zero return code: %d\n",
                executable, retval);
            exit(retval);
        }
    }
}

#define call_handler(executable) \
    _call_handler(executable, SBIN_PATH(executable))

int main() {
    signal(SIGINT, propagator);
    signal(SIGTERM, propagator);

    call_handler("swapoff");
    call_handler("swapon");

    return 0;
}
