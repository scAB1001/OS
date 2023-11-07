// Import necessary system headers from XV6 for types, user-level operations and file control
#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

// Define constants for buffer size and the maximum number of arguments
#define MAX_BUF 256
#define MAX_ARGS 32

// Function to check if a character is a white space
// Checks for space, horizontal tab, and newline characters
// XV6 may not support vertical tab, form feed, or carriage return
int is_whitespace(char c)
{
    // Returns true (1) if 'c' is a whitespace character; false (0) otherwise
    return (c == ' ' || c == '\t' || c == '\n');
}

// Function to duplicate a file descriptor to a given file descriptor number
// If the file descriptors match, it returns the same descriptor
// Otherwise, it attempts to duplicate the 'oldfd' to 'newfd'
int dup2(int oldfd, int newfd)
{
    // Check if the file descriptors are identical; if so, return the new file descriptor
    if (oldfd == newfd)
        return newfd;
    // If they are not the same, the new file descriptor is closed if it was previously open
    close(newfd);
    // The old file descriptor is then duplicated using the dup system call
    // The lowest-numbered unused descriptor is used for the duplicate
    return dup(oldfd);
}

// Function to prompt for and read a command line input
// It writes a prompt to stderr, clears the buffer, and reads a line of input
// If the input is EOF or empty, it returns -1 to indicate end of file/no input
int get_cmd(char *buf, int nbuf)
{
    // Write the command prompt ">>> " to stderr (file descriptor 2)
    fprintf(2, ">>> ");
    // Clear the buffer to prevent reading of any leftover data
    memset(buf, 0, nbuf);
    // Read a line of input from stdin into the buffer
    gets(buf, nbuf);
    // Check if the buffer is empty (first character is null) indicating EOF
    if (buf[0] == 0)
        return -1; // Return -1 to indicate EOF or no input was read
    // Return 0 to indicate that a command was successfully read
    return 0;
}

// Function to parse the input buffer into an array of argument strings
// Separates based on whitespace, handles maximum number of arguments, and tracks the argument count
void tokenize(char *buf, char *argv[], int argv_max, int *argc_out)
{
    // Pointer to traverse the buffer
    char *p = buf;
    // Initialize argument count
    int argc = 0;

    // Loop to process each argument up to the maximum allowed
    while (argc < argv_max)
    {
        // Skip over any leading whitespace before an argument
        while (is_whitespace(*p))
            p++;
        // If the current pointer is at the end of the string, break out of the loop
        if (*p == 0)
            break;
        // Set the current argument to the non-whitespace character's pointer
        argv[argc++] = p;
        // Move the pointer past the current argument
        while (!is_whitespace(*p) && *p != 0)
            p++;
        // If end of the string is reached during argument scan, break the loop
        if (*p == 0)
            break;
        // Null terminate the current argument and move the pointer to the next character
        *p++ = 0;
    }

    // Check if the number of arguments exceeds the maximum allowed
    if (argc >= argv_max)
    {
        // If so, print an error message to stderr
        fprintf(2, "Too many arguments\n");
        // Set argc to -1 to indicate an error in argument parsing
        argc = -1;
    }
    else
    {
        // Otherwise, null-terminate the array of arguments
        argv[argc] = 0;
    }

    // If a pointer to store the number of arguments was provided, set it to argc
    if (argc_out != 0)
    {
        *argc_out = argc;
    }
}

// Function to handle IO redirection in a command
// It modifies the command's argument vector to execute without the redirection operators
// and updates file descriptor pointers to point to the new file descriptors if redirection is specified
void redirect_io(char **argv, int *fdIn, int *fdOut)
{
    // Iterate over the argument vector looking for redirection symbols
    for (int i = 0; argv[i]; i++)
    {
        // Check if the current argument is an output redirection symbol ">"
        if (strcmp(argv[i], ">") == 0)
        {
            // Nullify the redirection argument to terminate the command arguments before execution
            argv[i] = 0;
            // Ensure that there is a filename following the redirection symbol
            if (argv[i + 1])
            {
                // Open the output file with write permissions, create if it does not exist
                *fdOut = open(argv[i + 1], O_WRONLY | O_CREATE);
                // Check if the file descriptor is valid, indicating successful opening of the file
                if (*fdOut < 0)
                {
                    // Print an error message if the file could not be opened and exit the process
                    printf("Cannot open file %s\n", argv[i + 1]);
                    exit(1);
                }
            }
            else
            {
                // If no filename is provided after ">", print an error message and exit the process
                printf("Output redirection requires a filename\n");
                exit(1);
            }
            // Move past the filename to continue parsing potential further arguments
            i++;
        }
        // Check if the current argument is an input redirection symbol "<"
        else if (strcmp(argv[i], "<") == 0)
        {
            // Nullify the redirection argument to terminate the command arguments before execution
            argv[i] = 0;
            // Ensure that there is a filename following the redirection symbol
            if (argv[i + 1])
            {
                // Open the input file with read permissions
                *fdIn = open(argv[i + 1], O_RDONLY);
                // Check if the file descriptor is valid, indicating successful opening of the file
                if (*fdIn < 0)
                {
                    // Print an error message if the file could not be opened and exit the process
                    printf("Cannot open file %s\n", argv[i + 1]);
                    exit(1);
                }
            }
            else
            {
                // If no filename is provided after "<", print an error message and exit the process
                printf("Input redirection requires a filename\n");
                exit(1);
            }
            // Move past the filename to continue parsing potential further arguments
            i++;
        }
    }
}

// Function to determine if there is a pipe '|' in the command arguments
// and to set up and execute the piping mechanism if found
int pipe_found(char **argv)
{
    // Flag to indicate whether a pipe is found in the command
    int piped = 0;
    // File descriptors array for pipe: fd_pipe[0] is for reading, fd_pipe[1] for writing
    int fd_pipe[2];

    // Loop through the arguments to find a pipe symbol
    for (int i = 0; argv[i]; i++)
    {
        // Check if the current argument is a pipe symbol "|"
        if (strcmp(argv[i], "|") == 0)
        {
            // If a pipe is found, indicate it with the piped flag
            piped = 1;
            // Null-terminate the arguments at the pipe symbol to separate the commands
            argv[i] = 0;
            // Get the second part of the command starting from the argument after the pipe symbol
            char **argv2 = &argv[i + 1];

            // Attempt to create a pipe
            if (pipe(fd_pipe) < 0)
            {
                // If pipe creation fails, output an error message and exit
                fprintf(2, "pipe failed\n");
                exit(1);
            }

            // Create the first child process to handle the first part of the pipe
            int pid1 = fork();
            if (pid1 == 0)
            {
                // First child process executes here:
                // Close the unused read end of the pipe
                close(fd_pipe[0]);
                // Redirect standard output to the pipe's write end
                dup2(fd_pipe[1], 1);
                // Close the write end of the pipe, now duplicated to standard output
                close(fd_pipe[1]);
                // Execute the first command
                exec(argv[0], argv);
                // If exec fails, output an error message and exit
                fprintf(2, "exec %s failed\n", argv[0]);
                exit(1);
            }

            // Create the second child process to handle the second part of the pipe
            int pid2 = fork();
            if (pid2 == 0)
            {
                // Second child process executes here:
                // Close the unused write end of the pipe
                close(fd_pipe[1]);
                // Redirect standard input to the pipe's read end
                dup2(fd_pipe[0], 0);
                // Close the read end of the pipe, now duplicated to standard input
                close(fd_pipe[0]);
                // Execute the second command
                exec(argv2[0], argv2);
                // If exec fails, output an error message and exit
                fprintf(2, "exec %s failed\n", argv2[0]);
                exit(1);
            }

            // Parent process closes both ends of the pipe
            // No need for the parent to communicate with the pipe
            close(fd_pipe[0]);
            close(fd_pipe[1]);
            // Parent waits for both child processes to finish
            wait(0);
            wait(0);

            // After handling the pipe, break from the loop
            // because the remaining arguments have been passed to the second command
            break;
        }
    }
    // Return the piped flag to indicate whether a pipe was handled or not
    return piped;
}

// Function to execute a command without a pipe
// It handles redirection if necessary and then executes the command
void pipe_not_found(char **argv, int *fdIn, int *fdOut)
{
    // Fork to create a new process
    int pid = fork();
    if (pid == 0)
    {
        // Child process:
        // If input redirection is specified, replace standard input with the input file
        if (*fdIn != -1)
        {
            dup2(*fdIn, 0);
            // Close the original file descriptor as it's no longer needed
            close(*fdIn);
        }
        // If output redirection is specified, replace standard output with the output file
        if (*fdOut != -1)
        {
            dup2(*fdOut, 1);
            // Close the original file descriptor as it's no longer needed
            close(*fdOut);
        }
        // Execute the command
        exec(argv[0], argv);
        // If exec fails, output an error message and exit
        printf("exec %s failed\n", argv[0]);
        exit(1);
    }
    else if (pid > 0)
    {
        // Parent process:
        // Wait for the child process to complete
        wait(0);
        // Close any file descriptors if they have been used for redirection
        if (*fdIn != -1)
            close(*fdIn);
        if (*fdOut != -1)
            close(*fdOut);
    }
    else
    {
        // If fork fails, output an error message and exit
        printf("fork failed\n");
        exit(1);
    }
}

// Entry point of the shell program
int main(void)
{
    // Static buffer to store the command input by the user
    static char buf[MAX_BUF];
    // Array of pointers to hold the tokenized command and its arguments
    char *argv[MAX_ARGS];
    // Variable to hold the number of arguments
    int argc;

    // Infinite loop to keep the shell running
    while (1)
    {
        // Read a command from the user input
        // If reading the command fails, skip the current iteration
        if (get_cmd(buf, sizeof(buf)) < 0)
            continue; // Skip if no command is entered

        // Tokenize the command string into arguments
        tokenize(buf, argv, sizeof(argv) / sizeof(argv[0]), &argc);
        // If tokenization fails (e.g., if argc is negative), skip the current iteration
        if (argc < 0)
            continue; // Skip if there was an error during tokenization

        // Check if the first argument is the "cd" command
        if (strcmp(argv[0], "cd") == 0)
        {
            // If there is no second argument for the "cd" command, print an error message
            if (argv[1] == 0)
            {
                fprintf(2, "cd missing argument\n");
            }
            else
            {
                // Try to change the directory to the path given in the second argument
                // If changing directory fails, print an error message
                if (chdir(argv[1]) < 0)
                {
                    fprintf(2, "cd: failed to change directory to %s\n", argv[1]);
                }
            }
            // After handling "cd", continue to the next iteration
            continue; // "cd" is handled in the shell process
        }

        // Initialize file descriptors for input and output redirection to invalid value
        int fdIn = -1, fdOut = -1;
        // Call a function to check for redirection symbols in the command and set up redirection
        redirect_io(argv, &fdIn, &fdOut);

        // Flag to indicate if a pipe symbol was found in the command
        int piped = 0;
        // Check for pipe in the command and execute the necessary commands if a pipe is found
        piped = pipe_found(argv);

        // If there was no pipe symbol found in the command
        if (!piped)
        {
            // Execute the command without a pipe, including handling any redirection
            pipe_not_found(argv, &fdIn, &fdOut);
        }
        // Loop back to the beginning to process the next command
    }
    // Exit the shell with a status code of 0 (successful termination)
    exit(0);
}
