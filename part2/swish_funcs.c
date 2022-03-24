#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "string_vector.h"
#include "swish_funcs.h"

#define MAX_ARGS 10

/*
 * Helper function to close all file descriptors in an array. You are
 * encouraged to use this to simplify your error handling and cleanup code.
 * fds: An array of file descriptor values (e.g., from an array of pipes)
 * n: Number of file descriptors to close
 */
int close_all(int *fds, int n) {
    int ret_val = 0;
    for (int i = 0; i < n; i++) {
        if (close(fds[i]) == -1) {
            perror("close");
            ret_val = 1;
        }
    }
    return ret_val;
}

/*
 * Helper function to run a single command within a pipeline. You should make
 * make use of the provided 'run_command' function here.
 * tokens: String vector containing the tokens representing the command to be
 * executed, possible redirection, and the command's arguments.
 * pipes: An array of pipe file descriptors.
 * n_pipes: Length of the 'pipes' array
 * in_idx: Index of the file descriptor in the array from which the program
 *         should read its input, or -1 if input should not be read from a pipe.
 * out_idx: Index of the file descriptor int he array to which the program
 *          should write its output, or -1 if output should not be written to
 *          a pipe.
 * Returns 0 on success or 1 on error.
 */
int run_piped_command(strvec_t *tokens, int *pipes, int n_pipes, int in_idx, int out_idx) {
    // TODO Complete this function's implementation
    return 0;
}

int run_pipelined_commands(strvec_t *tokens) {
    // TODO Complete this function's implementation
    return 0;
}
