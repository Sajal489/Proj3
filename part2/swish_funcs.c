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
 * out_idx: Index of the file descriptor in the array to which the program
 *          should write its output, or -1 if output should not be written to
 *          a pipe.
 * Returns 0 on success or 1 on error.
 */
int run_piped_command(strvec_t *tokens, int *pipes, int n_pipes, int in_idx, int out_idx) {
    // TODO Complete this function's implementation
    if (close_all(pipes, n_pipes*2 - 2) == 1) {
        perror("close_all");
        return 1;
    }
    if(out_idx != -1 && in_idx == -1){
        if (close(pipes[out_idx - 1]) == -1) {
            perror("close");
            return 1;
        }
        if (dup2(pipes[out_idx], STDOUT_FILENO) == -1) {
            perror("dup2");
            return 1;
        }
        if (close(pipes[out_idx]) == -1) {
            perror("close");
            return 1;
        }
    }else if(out_idx == -1 && in_idx != -1){
         if(close(pipes[in_idx]) == -1) {
             perror("closed failed");
             return 1;
         }
         if (dup2(pipes[in_idx - 1], STDIN_FILENO) == -1) {
             perror("dup2");
             return 1;
         }
         if (close(pipes[in_idx - 1]) == -1) {
             perror("close");
             return 1;
         }
    }else if(out_idx != -1 && in_idx != -1){
        if (close(pipes[out_idx - 1]) == -1) {
            perror("close failed");
            return 1;
        }
        if(close(pipes[in_idx]) == -1) {
            perror("closed failed");
            return 1;
        }
        if (dup2(pipes[out_idx], STDOUT_FILENO) == -1) {
            perror("dup2");
            return 1;
        }
        if (dup2(pipes[in_idx-1], STDIN_FILENO) == -1) {
            perror("dupe2");
            return 1;
        }
    }else return 1;
    
    if (run_command(tokens) == 1) {
        perror("run_cmd");
        return 1;
    }
    return 0;
}

int run_pipelined_commands(strvec_t *tokens) {
    // TODO Complete this function's implementation
    int n_pipes = 0;
    int index = 0;
    for(int i = 0; i < tokens->length; i++){
        char *temp = strvec_get(tokens, i);
        if (temp == NULL) {
            printf("strvec get failed");
            return 1;
        }
        if(strcmp(temp, "|") == 0) n_pipes++;
    }

    strvec_t arr[n_pipes+1];
    strvec_t *temp = malloc(sizeof(strvec_t));
    if (temp == NULL) {
        perror("malloc");
        return 1;
    }
    if (strvec_init(arr) == 1) {
        printf("strvec init failed\n");
        return 1;
    }
    if (strvec_init(temp) == 1) {
        printf("strvec init failed\n");
        return 1;
    }

    for(int i = 0; i < tokens->length; i++){
        char *token = strvec_get(tokens, i);
        if (token == NULL) {
            printf("strvec get failed");
            return 1;
        }
        if(strcmp(token, "|") == 0){
            arr[index++] = *temp;
            temp = malloc(sizeof(strvec_t));
            if (temp == NULL) {
                printf("2nd malloc failed\n");
                return 1;
            }
            if (strvec_init(temp) == 1) {
                printf("strvec init failed\n");
                return 1;
            }

        }else{
            token = strvec_get(tokens, i);
            if (token == NULL) {
                printf("strvec get failed");
                return 1;
            }
            if (strvec_add(temp, token) == 1) {
                printf("strvec_add");
                return 1;
            }
        }
    }
    arr[index] = *temp;

    int *pipes = malloc(2 * (n_pipes) * sizeof(int));
    if (pipes == NULL) {
        perror("malloc");
        return 1;
    }

    for(int i = 0; i < n_pipes; i++){
        if (pipe(pipes + 2*i) == 1) {
            perror("pipe");
            free(pipes);
            return 1;
        }
    }


    for(int i = 0; i < n_pipes+1; i++){
        pid_t pid = fork();
        if(pid == 0){
            if(i > 0 && i < n_pipes){
                if (run_piped_command(arr + i, pipes, i, 2*i-1, 2*i+1) == 1) {
                    printf("run piped cmd");
                    free(pipes);
                    exit(1);
                }
            }else if(i == 0){
                if (run_piped_command(arr + i, pipes, i, -1, 2*i+1) == 1) {
                    printf("run piped cmd");
                    free(pipes);
                    exit(1);
                }
            }else{
                if (run_piped_command(arr + i, pipes, i, 2*i-1, -1) == 1) {
                    printf("run piped cmd");
                    free(pipes);
                    exit(1);
                }
            }    
        }else if(pid > 0){
            //parent
        }else{
            perror("failed to fork child");
            free(pipes);
            exit(1);
        }
    }
    if (close_all(pipes, 2*n_pipes) == 1) {
        printf("failed to close all pipes");
        free(pipes);
        return 1;
    }
      int status = 0;
      for(int i = 0; i < n_pipes+1; i++){
          if (wait(&status) == -1) {
              perror("wait");
              return 1;
          }
      }
      free(pipes);
    return 0;
}
