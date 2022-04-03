#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

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
    if(out_idx != -1 && in_idx == -1){
        dup2(pipes[out_idx], STDOUT_FILENO);
    }else if(out_idx == -1 && in_idx != -1){
         dup2(pipes[in_idx], STDIN_FILENO);
    }else if(out_idx != -1 && in_idx != -1){
        dup2(pipes[out_idx], STDOUT_FILENO);
        dup2(pipes[in_idx], STDIN_FILENO);
    }else return 1;
    run_command(tokens);
    return 0;
}

int run_pipelined_commands(strvec_t *tokens) {
    // TODO Complete this function's implementation
    int n_pipes = 0;
    int index = 0;
    for(int i = 0; i < tokens->length; i++){
        if(strcmp(strvec_get(tokens, i), "|") == 0) n_pipes++;
    }

    strvec_t arr[n_pipes+1];
    strvec_t *temp = malloc(sizeof(strvec_t));
    strvec_init(arr);
    strvec_init(temp);

    for(int i = 0; i < tokens->length; i++){
        if(strcmp(strvec_get(tokens, i), "|") == 0){
            arr[index++] = *temp;
            temp = malloc(sizeof(strvec_t));
            strvec_init(temp);

        }else{
            strvec_add(temp, strvec_get(tokens, i));
        }
    }
    arr[index] = *temp;

    int *pipes = malloc(2 * (n_pipes) * sizeof(int));

    for(int i = 0; i < n_pipes; i++){
        pipe(pipes + 2*i);
    }


    for(int i = 0; i < n_pipes+1; i++){
        pid_t pid = fork();
        if(pid == 0){
            if(i > 0 && i < n_pipes){
               //i > 0 && i < n_pipes ? 2*(i-1) : i == 0 ? -1 : 2*i
                run_piped_command(arr + i, pipes, 2*n_pipes, 2*(i-1), 2*i+1);
            }else if(i == 0){
                run_piped_command(arr + i, pipes, 2*n_pipes, -1, 2*i+1);
            }else{
                run_piped_command(arr + i, pipes, 2*n_pipes, 2*i, -1);
            }     
        }else if(pid > 0){
            //parent
        }else{
            perror("failed to fork child");
            exit(1);
        }
    }

      close_all(pipes, 2*n_pipes);
      int status = 0;
      for(int i = 0; i < n_pipes+1; i++){
          wait(&status);
      }
      free(pipes);
    return 0;
}
