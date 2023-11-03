#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>


// Execute a command
void run_cmd(char **cmd)
{
    /*
    int pid;

    // Fork a child process
    if ((pid = fork()) < 0)
    {
        fprintf(1, "Fork failed\n");
        return;
    }

    if (pid == 0)
    {
        // Child process
        if (exec(*cmd, cmd) == -1)
        {
            printf("exec %s failed\n", *cmd);
        }
        else
        {
            printf("exec %s\n", *cmd);
        }

        exit(0);
    }
    else
    {
        // Parent process
        wait(0); // Wait for child process to finish
    }
    */
}

// Get the length of a string
int count_chars(char *str)
{
    int length = 0;
    while (*str != '\0')
    {
        length++;
        str++;
    }
    return length;
}

// Get the length of the array of strings
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

// Breaks when "' or '"
int is_quote(char c)
{
    return c == '\'' || c == '"';
}

// Act as echo.c
void echo(char **tokens)
{
    char temp;
    int i, j;

    printf("\n$ ");
    
    // Ignore cmd
    for (i = 1; i < count_strings(tokens); i++)
    {
        for (j = 0; j < count_chars(tokens[i]); j++)
        {
            if (!is_quote(tokens[i][j]))
            {
                printf("%c", tokens[i][j]);
            }
        }
        printf(" ");
    }
    printf("\n");
}

// Remove a final spaces from a string
void rm_trailing_whitespace(char *str)
{
    // - 1 as the actual last is '\0'
    int strLen = count_chars(str) - 1;
    char lastChar = str[strLen];

    if (lastChar == ' ')
    {
        str[strLen] = '\0';
    }
}

// Remove consecutive spaces from a string
void rm_consecutive_whitespace(char *str)
{
    char output[512];

    // Track consecutive spaces (single spaces eg "cd .." are allowed)
    int i, numSpaces = 0;

    // Iterate until a null terminator is reached
    for (i = 0; str[i]; i++)
    {
        // Copy cur char or prev non-space chars (not the first)
        if (str[i] != ' ' || (i > 0 && str[i - 1] != ' '))
        {
            output[i - numSpaces] = str[i];
        }
        else
        {
            numSpaces++;
        }
    }

    output[i - numSpaces] = '\0';

    // Copy the modified string back to str
    for (i = 0; output[i]; i++)
    {
        str[i] = output[i];
    }

    // Remove trailing (consecutive) whitespace
    str[i] = '\0';
}

// Group function to remove all possible excess whitespace
void rm_whitespace(char *str)
{
    rm_consecutive_whitespace(str);
    rm_trailing_whitespace(str);
}

void display_tokens(char **tokens)
{
    printf("Tokens:\t\t");
    for (int i = 0; i < count_strings(tokens); i++)
    {
        printf("'%s'\t", tokens[i]);
    }

    /*
    int i = 0;
    while (tokens[i] != NULL)
    {
        printf("'%s'\t", tokens[i]);
        i++;
    }
    */
    printf("\n");
}

// Tokenize a cleaned string input
/* DOES NOT WORK FOR: echo "Hello World" -> {echo, "Hello, World"} 
 *  instead of {echo, "Hello World"}. 
 *  Use non-simple method trick for quotes.
 */
void tokenize_string(char *str, char **tokens)
{
    int i = 0;
    while (*str != '\0')
    {
        // Store the start of the token
        tokens[i] = str;

        // Find the end of the token
        while (*str != ' ' && *str != '\0')
        {
            str++;
        }

        // End the token when a space is found
        if (*str == ' ')
        {
            *str = '\0';
            str++;
        }

        // Proceed to the next token
        i++;
    }

    // Ensure the last element is NULL to mark the end of tokens
    tokens[i] = NULL;
    display_tokens(tokens);
}

void cd(char **tokens)
{
    // Verify valid cd cmd
    if (count_strings(tokens) == 2)
    {
        *tokens++;
        if(-1 < 0)//chdir(tokens)
        {
            printf("cd: %s: No such file or directory.\n", *tokens);
        }
    }
    else
    {
        printf("Please include a path.\n");
    }
}

/* Part 3: Input/Output redirection (6 Marks)
 * Implement Input/Output redirection. 
 * Your shell should be able to handle two-element redirection.
 * For example:
 *   $ echo "Hello world" > temp    |   Open to write the left
 *   $ cat < temp                   |   Open to read the right
 * 
*/



// Finds the first occurrence of that character 
int contains_char_xtra(char **strings, char target, int *strPos, int *charPos)
{
    // Start value for indices (non-existent to start)
    *strPos = -1;
    *charPos = -1;

    // str and char counters, flag to indicate if inside quotes
    int i = 0, j, inQuotes;

    // Loop through each string in arr of strings
    while (strings[i] != NULL)
    {
        char *curStr = strings[i];
        j = 0;
        inQuotes = 0;

        // Loop through chars in current string
        while (curStr[j] != '\0')
        {
            // Check for a valid, non-quote match (for '<' or '>')
            if (j == 0 && curStr[j] == target && !inQuotes)
            {
                *strPos = i;
                *charPos = j;
                return 1;
            }
            if (is_quote(curStr[j]))
            {
                // Toggle flag
                inQuotes = !inQuotes;
            }
            j++;
        }
        i++;
    }

    return 0;
}

// Simpler
int contains_char(char **strings, char target, int *strPos)
{
    /* Need to find a '<' or '>' alone, as the first index of 
     *  that current string's token else ignore.
     * Store that string index in the arr, for later processing.
    */
    *strPos = -1;
    int i = 0, j;

    // Loop through each string in arr of strings
    while (strings[i] != NULL)
    {
        char *curStr = strings[i];
        j = 0;

        // Loop through chars in current string
        while (curStr[j] != '\0')
        {
            // Check for a valid, non-quote match (for '<' or '>')
            if (j == 0 && curStr[j] == target)
            {
                *strPos = i;
                return 1;
            }
            j++;
        }
        i++;
    }

    return 0;
}

// Group method for finding a target character in an arr of strings
int search_for(char target, char** str)
{
    int strPos, charPos;
    // print first string
    //printf("\n> ' %s '\n", *str);
    //if (contains_char_xtra(str, target, &strPos, &charPos))
    if (contains_char(str, target, &strPos))
    {
        //printf("Found '%c' at string[i]: %d, character[i]: %d.\n", target, strPos, charPos);
        printf("\nFound '%c' at string[i]: %d.\n", target, strPos);
    }
    else
    {
        printf("\nDid not find '%c' character in the array of strings.\n", target);
    }

    return 0;
}

void check_direction(char **tokens)
{
    
}

int brev()
{
    // open() returns a file descriptor file_desc to a the file "dup.txt" here"

    int file_desc = open("dup.txt", O_WRONLY | O_APPEND);

    if (file_desc < 0)
    {
        printf("Error opening the file\n");
    }

    // dup() will create the copy of file_desc as the copy_desc
    // then both can be used interchangeably.

    int copy_desc = dup(file_desc);

    // write() will write the given string into the file
    // referred by the file descriptors

    write(copy_desc, "This will be output to the file named dup.txt\n", 46);
    write(file_desc, "This will also be output to the file named dup.txt\n", 51);

    return 0;
}

int main(int argc, char *argv[])
{
    char input[512], *tokens[64];
    //search_for();

    
    while (1)
    {
        printf("\n------------------------------\n");
        printf(">>> ");

        fgets(input, sizeof(input), stdin);
        input[count_chars(input) - 1] = '\0';

        rm_whitespace(input);
        printf("input:\t'%s'\n", input);

        tokenize_string(input, tokens);
        //search_for('<', tokens);

        if (strcmp(input, "exit") == 0)
        {
            printf("\nYou left the shell.\n");
            printf("\n------------------------------\n\n");
            break; // Exit the shell
        }
        else if (strcmp(input, "cd") == 0)
        {
            cd(tokens);
            break;
        }
        else if (strcmp(input, "ls") == 0)
        {
            // Handle "ls" command
            // run_cmd(tokens);
            display_tokens(tokens);
        }
        else if (strcmp(tokens[0], "echo") == 0)
        {
            // Handle "echo" command
            echo(tokens); 
            //brev();
        }
        else
        {
            // Handle shell commands
            //run_cmd(tokens);
            display_tokens(tokens);
        }
        printf("\n------------------------------\n\n");
    }
    /**/

    exit(0);
}