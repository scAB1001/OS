#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include <stddef.h>

/*
#define O_RDONLY 0x000
#define O_WRONLY 0x001
#define O_RDWR 0x002
#define O_CREATE 0x200
#define O_TRUNC 0x400
*/

#define MAX_BUF_SIZE 128
#define MAX_TOKENS 64

int count_strings(char **str)
{
    int count = 0;
    while (*str != NULL)
    {
        count++;
        str++;
    }
    return count;
}

void free_tokens(char **tokens)
{
    for (int i = 0; tokens[i] != NULL; i++)
    {
        free(tokens[i]);
    }
}

void clear_token_array(char **tokens, int size)
{
    for (int i = 0; i < size; i++)
    {
        tokens[i] = NULL;
    }
}

// Prompt the user for an input
void read_input(char *str, int max_size)
{
    printf(">>> ");
    // Ensure there's room for null terminator
    int bytesRead = read(0, str, max_size - 1);
    if (bytesRead > 0)
    {
        str[bytesRead - 1] = '\0';
    }

    //printf("\nIN: [%s]\n", str);
}

// Tokenize the input string, ignoring excess whitespace
void tokenize(char *str, char **tokens)
{
    int i = 0;

    // Skip any initial spaces
    while (*str == ' ')
    {
        str++;
    }

    while (*str && i < MAX_TOKENS - 1)
    {
        // Assign token start
        tokens[i++] = str;

        // Move str to the end of the current token
        while (*str != ' ' && *str != '\0')
        {
            str++;
        }
        // If end of string, break out of loop
        if (*str == '\0')
        {
            break;
        }

        // Null-terminate the current token and move to the next one
        *str++ = '\0';

        // Skip any consecutive spaces
        while (*str == ' ')
        {
            str++;
        }
    }

    // Null-terminate the array of tokens
    tokens[i] = NULL; 
    //print_tokens(tokens);
}

// Act as cd.c
void cd(char **tokens)
{
    // Verify valid cd cmd
    if (count_strings(tokens) == 2)
    {
        if (chdir(tokens[1]) < 0)
        {
            printf("cd: %s: No such file or directory.\n", tokens[1]);
        }
    }
    else
    {
        printf("Please include a path.\n");
    }
}

// Execute a command
void run_cmd(char **cmd)
{
    int pid = fork();

    if (pid == 0)
    { // Child process
        if (exec(cmd[0], cmd) < 0)
        {
            printf("exec %s failed\n", cmd[0]);
            exit(1);
        }
    }
    else if (pid > 0)
    { // Parent process
        wait(0);
    }
    else
    {
        printf("Fork failed\n");
        return;
    }
    /**/
}


int main()
{
    char input[MAX_BUF_SIZE];
    char *tokens[MAX_TOKENS];

    while (1)
    {
        // Clear buffer?
        read_input(input, MAX_BUF_SIZE);

        // Clear the tokens array?
        //clear_token_array(tokens, MAX_TOKENS);
        tokenize(input, tokens);

        if (strcmp(tokens[0], "exit") == 0)
        {
            printf("Exiting the shell.\n");
            exit(0);
        }
        else if (strcmp(tokens[0], "cd") == 0)
        {
            cd(tokens);
        }
        else
        {
            run_cmd(tokens);
        }

        free_tokens(tokens);
    }
    return 0;
}