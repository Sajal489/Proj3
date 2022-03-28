#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_WORD_LEN 25

/*
 * Counts the number of occurrences of words of different lengths in a text file and
 * stores the results in an array.
 * file_name: The name of the text file from which to read words
 * counts: An array of integers storing the number of words of each possible length.
 *         counts[0] is the number of 1-character words, counts [1] is the number
 *         of 2-character words, and so on.
 * max_len: The maximum length of any word, and also the length of 'counts'.
 * Returns 0 on success or 1 on error.
 */
int count_word_lengths(const char *file_name, int *counts, int max_len) {
    FILE *fh = fopen(file_name, "r");
    if(fh == NULL)return 1;
    char words[MAX_WORD_LEN];
    while(fscanf(fh, "%s", words) != EOF){ // does this account for white space?
        counts[strlen(words) - 1]++;
        memset(words, 0, sizeof(words)); // error check right?
    }
    fclose(fh);
    return 0;
    
}

/*
 * Processes a particular file (counting the number of words of each length)
 * and writes the results to a file descriptor.
 * This function should be called in child processes.
 * file_name: The name of the file to process.
 * out_fd: The file descriptor to which results are written
 * Returns 0 on success or 1 on error
 */
int process_file(const char *file_name, int out_fd) {
    int *counts = malloc(sizeof(int) * MAX_WORD_LEN);
    for (int i = 0; i < MAX_WORD_LEN; i++) {
        counts[i] = 0;
    }
    if (counts == NULL) {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }
    if (count_word_lengths(file_name, counts, MAX_WORD_LEN) == 1) {
        perror("count word lenghts");
        free(counts);
        return 1;
    }
    if (write(out_fd, counts, sizeof(int)*MAX_WORD_LEN) == -1) {
        perror("write");
        free(counts);
        return 1;
    }
    free(counts);
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 1) {
        // No files to consume, return immediately
        return 0;
    }

    // TODO Create a pipe for each child process
    int *pipe_fds = malloc(2 * (argc - 1) * sizeof(int));
    if(pipe_fds == NULL){
        fprintf(stderr, "malloc failed\n");
        return -1;
    }
    // TODO Fork a child to process all specified files (names are argv[1], argv[2], ...)
    for(int i = 0; i < argc - 1; i++){
        if (pipe(pipe_fds + 2*i) == -1) {
            perror("pipe");
            free(pipe_fds);
            return -1;
        }
        pid_t child_pid = fork();
        // error checking here for ^^
        if (child_pid == 0) { // child process
            for (int j = 0; j <= i; j++) { // close read end of all previous pipes
                close(pipe_fds[2*j]);
            }
            if (process_file(argv[i+1], pipe_fds[2 * i + 1]) == 1) { // count word lengths for given file
                perror("process_file");
                free(pipe_fds);
                exit(1); // should exit code 1 if in child fails
            }
            close(pipe_fds[2 * i + 1]); // close pipe child just wrote to
            free(pipe_fds);
            exit(0);
        } else {
            close(pipe_fds[2*i+1]);
        } 
    }

    // TODO Aggregate all the results together by reading from pipes in the parent
    int results[MAX_WORD_LEN]; // final output 
    memset(results, 0, sizeof(results)); // error check?
    int temp_results[MAX_WORD_LEN]; // read in the counts arrays and store them in here. need to initialize w/ 0s
    for (int i = 0; i < argc - 1; i++) {
        memset(temp_results, 0, sizeof(temp_results));
        if (read(pipe_fds[2*i], &temp_results, sizeof(int)*MAX_WORD_LEN) == -1) {
            free(pipe_fds);
            perror("read");
            return -1;
        }
        for (int j = 0; j < MAX_WORD_LEN; j++) {
            results[j] += temp_results[j];
        }
    }
    // TODO Change this code to print out the total count of words of each length
    for (int i = 1; i <= MAX_WORD_LEN; i++) {
        printf("%d-Character Words: %d\n", i, results[i-1]);
    }
    return 0;
}




