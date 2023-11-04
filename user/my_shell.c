#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include <stddef.h>
// #include <fcntl.h>

// Execute a command
void run_cmd(char **cmd)
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

// Breaks when "' or '"
int is_quote(char c)
{
	return c == '\'' || c == '"';
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
	printf("\n");
}

// Tokenize a cleaned string input
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
		// For some reasons, char* x++; does not work in xv6
		char* path = tokens[1];
		if (chdir(path) < 0)
		{
			printf("cd: %s: No such file or directory.\n", path);
		}
		else
		{
			printf("You have moved to %s.\n", path);
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
 *   $ echo "Hello world" > temp    F|   Open to write the left
 *   $ cat < temp                   |   Open to read the right
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
int search_for(char target, char **str)
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
	//*tokens++;
	if ((fd = open(*tokens, 0)) < 0)
	{
		printf("Cannot open %s\n", *tokens);
		exit(0);
	}

	// Open destination file for writing
	//*tokens++;
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
	// Replace '\n' with '\0'
	int bytesRead = read(0, cmd, sizeof(cmd));
	cmd[bytesRead - 1] = '\0';

	rm_whitespace(cmd);
	tokenize(cmd, tokens);
}

int other_programs(char **tokens)
{
	//int stringIndex;
	
	if (strcmp(*tokens, "exit") == 0)
	{
		printf("\nYou left the shell.\n");
		return 1;
	}
	else if (strcmp(*tokens, "cd") == 0)
	{
		cd(tokens);
	}
	else
	{ // Run shell commands
		run_cmd(tokens);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	char cmd[512], *tokens[512];

	while (1)
	{
		prompt_user(cmd, tokens);

		if (other_programs(tokens))
		{ // Exit
			break;
		}
	}

	exit(0);
}
