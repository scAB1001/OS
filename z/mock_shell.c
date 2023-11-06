#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
//#include <fcntl.h> // Includes 0_

#define O_RDONLY 0x000
#define O_WRONLY 0x001
#define O_RDWR 0x002
#define O_CREATE 0x200
#define O_TRUNC 0x400

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
        printf("<%s>\t", tokens[i]);
    }

    printf("\n");
}

// Tokenize a cleaned string input
/* DOES NOT WORK FOR: echo "Hello World" -> {echo, "Hello, World"} 
 *  instead of {echo, "Hello World"}. 
 *  Use non-simple method trick for quotes.
 */
void tokenize(char *str, char **tokens)
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
 * For example:
 *   $ echo "Hello world" > temp    |   Open to write the left
 *   $ cat < temp                   |   Open to read the right
 * 
*/

// Finds the first occurrence of that character 
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
    int strPos;
    if (contains_char(str, target, &strPos))
    {
        printf("\nFound '%c' at string[i]: %d.\n", target, strPos);
    }
    else
    {
        printf("\nDid not find '%c' character in the array of strings.\n", target);
        printf("strPos: %d.\n", strPos);
    }

    return strPos;
}

void check_direction(char **tokens)
{
    
}

int elem_redirection(char *str, char **tokens)
{
    char buf[512];
    int fd;

    if (count_strings(tokens) != 3)
    {
        printf("Usage: %s <source file> <destination file>\n", str);
        exit(0);
    }

    // Open source file for reading
    *tokens++;
    if ((fd = open(*tokens, 0)) < 0)
    {
        printf("Cannot open %s\n", *tokens);
        exit(0);
    }

    // Open destination file for writing
    *tokens++;
    if ((fd = open(*tokens, O_CREATE | O_WRONLY)) < 0)
    {
        printf("Cannot open %s\n", *tokens);
        exit(0);
    }

    // Read from source and write to destination
    int bytesRead;
    while ((bytesRead = read(fd, buf, sizeof(buf))) > 0)
    {
        write(1, buf, bytesRead);  // Print to console
        write(fd, buf, bytesRead); // Write to destination file
    }

    // Close files
    close(fd);

    exit(0);
}

void prompt_user(char *cmd, char **tokens)
{
    printf("\n>>> ");

    fgets(cmd, 512, stdin);
    cmd[count_chars(cmd) - 1] = '\0';

    rm_whitespace(cmd);
    tokenize(cmd, tokens);
}

int other_programs(char **tokens)
{
    if (strcmp(*tokens, "exit") == 0)
    {
        printf("You left the shell.\n");
        return 1;
    }
    else if (strcmp(*tokens, "cd") == 0)
    {
        cd(tokens);
    }
    else if (strcmp(tokens[0], "echo") == 0)
    {
        echo(tokens);
    }

    return 0;
}

/*
    # Read
    cat > temp.txt

    # Store 
    cat file1.txt file2.txt > temp.txt

*/

void filter_dir(char* str, char** tokens)
{
    int numStrings = count_strings(tokens);
    // Index of the direction character in tokens.
    int dirIndex = search_for('>', tokens);
    
    if (dirIndex != -1)
    {
        // Create a new str arr for tokens before the '>', +1 for NULL 
        char *newTokens[dirIndex + 1];  

        // Copy tokens to newTokens
        for (int i = 0; i < dirIndex; i++)
        {
            newTokens[i] = tokens[i];
        }
        newTokens[dirIndex] = NULL;

        // Check file exists
        if (tokens[dirIndex + 1] != NULL)
        {
            // Open the file for writing
            int fd = open(tokens[dirIndex + 1], O_CREATE | O_WRONLY);
            if (fd < 0)
            {
                printf("Cannot open %s\n", tokens[dirIndex + 1]);
                exit(0);
            }

            // Write content to the file
            int i = dirIndex + 2;
            while (tokens[i] != NULL)
            {
                write(fd, tokens[i], strlen(tokens[i]));
                write(fd, "\n", 1);
                i++;
            }

            // Close the file descriptor
            close(fd);
        }
    }
}


int main(int argc, char *argv[])
{
    char cmd[512], *tokens[64];

    while (1)
    {
        prompt_user(cmd, tokens);

        //int stringIndex = search_for('<', tokens);

        if (other_programs(tokens))
        {
            break; // Exit
        }
        else
        {
            // Handle shell commands
            //display_tokens(tokens); // run_cmd(tokens);
        }
    }

    exit(0);
}

/* dup() explanation
1. Duplicating a File Descriptor:
In Unix-like systems, everything is treated as a file, including regular files, directories, devices, etc.
When you open a file, the operating system assigns a unique identifier to it called a file descriptor.

dup() is a system call that allows you to create a new file descriptor that points to the same underlying resource as an existing file descriptor.

2. Creating Multiple References:
Imagine you have opened a file and received a file descriptor, let's say fd1.
This descriptor is like a pointer to that file.
It allows you to read from, write to, and perform other operations on that file.

Now, if you use dup(fd1), it creates a new file descriptor, let's call it fd2, that points to the same file.
So, now you have two file descriptors (fd1 and fd2) that refer to the same file.

Both fd1 and fd2 can be used to perform operations on the file.
If you read from fd1, it's like reading from the file.
If you write to fd2, it's like writing to the same file.*/

/*MY SHELL.C LATEST VERSION BEFORE REMODEL - RANDID

    #include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include <stddef.h>
//#include <fcntl.h>

#define MAX_BUF_SIZE 512
#define MAX_TOKENS 512

/*
void free_tokens(char **tokens)
{
	for (int i = 0; tokens[i] != NULL; i++)
	{
		free(tokens[i]);
	}
	free(tokens);
}*/

// Execute a command
void run(char **str)
{
	int pid;

	// Fork a child process
	if ((pid = fork()) < 0)
	{
		printf("Fork failed\n");
		return;
	}

	if (pid == 0)
	{
		// Child process
		if (exec(str[0], str) == -1)
		{
			printf("exec %s failed\n", *str);
			exit(1);
		}

		//exit(0);
	}
	else
	{
		// Parent process
		wait(0); // Wait for child process to finish
	}
}

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

void display_tokens(char **tokens)
{
	printf("Tokens:");
	for (int i = 0; i < count_strings(tokens); i++)
	{
		printf("[%s] ", tokens[i]);
	}
	printf("\n");
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
	display_tokens(tokens);
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

int prompt_user(char *str, char **tokens)
{
	printf(">>> ");
	int bytesRead = read(0, str, sizeof(str));
	if (bytesRead < 0)
	{
		return 1;
	}

	// Replace '\n' with '\0'
	str[bytesRead - 1] = '\0';
	printf("\nInput: [%s]\n", str);
	return 0;
}

int exit_shell(char **tokens)
{
	if (strcmp(tokens[0], "exit") == 0)
	{
		printf("\nYou left the shell.\n");
		return 1;
	}
	return 0;
}

void other_programs(char **tokens)
{
	if (strcmp(tokens[0], "cd") == 0)
	{
		cd(tokens);
	}
	else
	{ // Run shell commands
		run(tokens);
	}
}

int main(int argc, char *argv[])
{
	char input[MAX_BUF_SIZE];
	char *tokens[MAX_TOKENS];

	while (1)
	{
		if (prompt_user(input, tokens))
		{ // Exit
			break;
		}

		// Handle Input	
		tokenize(input, tokens);

		if (exit_shell(tokens))
		{
			break;
		}

		// Handle new commands
		other_programs(tokens);
	}

	return 0;
}


*/