#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include <stddef.h>

// Execute a command
void run_cmd(char **cmd)
{
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

// Tokenize a cleaned string input
void tokenize_string(char *str, char **tokens)
{
	int i = 0;
	while (*str != '\0')
	{
		// Store the start of the token
		tokens[i] = str;

		// Find the end of the token
		while (*str != ' ' && *str != '\0')
			str++;

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
}

void display_tokens(char **tokens)
{
	printf("Tokens:\t\t");
	for (int i = 0; i < count_strings(tokens); i++)
		printf("'%s'\t", tokens[i]);

	/*
	int i = 0;
	while (tokens[i] != NULL)
	{
		printf("'%s'\t", tokens[i]);
		i++;
	}
	*/
}

int main(int argc, char *argv[])
{
	printf("--------------------\n");
	char input[512];
	char *tokens[64];

	while (1)
	{
		fprintf(0, ">>> ");

		// Using read system call to get input
		int bytesRead = read(0, input, sizeof(input));
		if (bytesRead <= 0)
		{
			break;
		}
		printf("\ninputV0:\t'%s'\n", input);

		// Replace '\n' with '\0'
		input[bytesRead - 1] = '\0';
		printf("inputV1:\t'%s'\n", input);

		rm_whitespace(input);
		printf("inputV2:\t'%s'\n", input);

		tokenize_string(input, tokens);
		display_tokens(tokens);
		printf("\n--------------------\n\n");

		if (strcmp(input, "exit") == 0)
		{
			break; // Exit the shell
		}
		else if (strcmp(input, "ls") == 0)
		{
			// Handle "ls" command
			run_cmd(tokens);
		}
		// The space is to allow for a destination
		else
		{
			run_cmd(tokens);
		}
	}

	exit(0);
}
