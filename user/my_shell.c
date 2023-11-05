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
