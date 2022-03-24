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
    char *words;
    while(fscanf(fh, "%s", words) != EOF){
        counts[strlen(words) - 1]++;
    }
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
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 1) {
        // No files to consume, return immediately
        return 0;
    }

    // TODO Create a pipe for each child process
    int *pipe_fds = malloc(2*argc * sizeof(int));
    if(pipe_fds == NULL){
        fprintf(stderr, "malloc failed\n");
        return -1;
    }
    // TODO Fork a child to process all specified files (names are argv[1], argv[2], ...)
    for(int i = 0; i < argc; i++){
        if (pipe(pipe_fds + 2*i) == -1) {
            perror("pipe");
            free(pipe_fds);
            return -1;
        }
        
    }

    // TODO Aggregate all the results together by reading from pipes in the parent
    // TODO Change this code to print out the total count of words of each length
    for (int i = 1; i <= MAX_WORD_LEN; i++) {
        printf("%d-Character Words: %d\n", i, -1);
    }
    return 0;
}




