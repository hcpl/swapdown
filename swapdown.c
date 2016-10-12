#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define exit_error(message) \
    perror(message); \
    exit(EXIT_FAILURE);

#define exit_error_if(cond, message) \
    if (cond) { \
        exit_error(message); \
    }


static volatile pid_t child_pid = 0;

static void propagator(int sig) {
    if (child_pid > 0) {
        kill(child_pid, sig);
    }
}

static void set_signal_handlers() {
    struct sigaction sa;
    size_t i;
    int signums[] = {
        SIGHUP,
        SIGINT,
        SIGQUIT,
        SIGTERM,
        SIGCONT,
        SIGTSTP,
    };

    sa.sa_handler = propagator;
    sigfillset(&sa.sa_mask);    /* Block all signals */
#ifdef SA_RESTART
    sa.sa_flags = SA_RESTART;
#endif

    for (i = 0; i < sizeof(signums) / sizeof(int); ++i) {
        if (sigaction(signums[i], &sa, NULL) == -1) {
            fprintf(stderr, "%2d ", signums[i]);
            exit_error("signal");
        }
    }
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
    set_signal_handlers();

    call_handler("swapoff");
    call_handler("swapon");

    return 0;
}
