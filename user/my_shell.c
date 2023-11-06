#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

/*#define O_RDONLY 0x000
#define O_WRONLY 0x001
#define O_RDWR 0x002
#define O_CREATE 0x200
#define O_TRUNC 0x400
*/

#define INPUT_SIZE 256

void is_whitespace(char c, int *result)
{
	*result = (c == ' ' || c == '\t' || c == '\n');
	// XV6 might not support them... || c == '\v' || c == '\f' || c == '\r');
}

void string_length(const char *str, int *length)
{
	const char *s;
	for (s = str; *s; ++s);
	*length = (s - str);
}

void string_copy(char *dest, const char *src, int n)
{
	while (n-- && (*dest++ = *src++));
	if (n >= 0)
	{
		*dest = '\0'; // Ensure null-termination
	}
}

void print_tokens(char **tokens, int token_count)
{
	printf("Tokens:");
	for (int i = 0; i < token_count; i++)
	{
		printf(" [%s]", tokens[i]);
	}
	printf("\t There are %d tokens.\n", token_count);
}

void malloc_token(char **tokens, int index, int length)
{
	// Allocate memory for the new token at the specified 'index',
	// +1 for null terminator.
	tokens[index] = malloc(length + 1);
	if (!tokens[index])
	{
		// If the memory allocation fails, print an error message and exit.
		printf("Memory allocation failed\n");
		exit(1);
	}
}

void allocate_tokens(char ***tokens, int new_count)
{
	// Allocate memory for the array of char* pointers with the correct size
	char **new_tokens = malloc(new_count * sizeof(char *));
	if (!new_tokens)
	{
		printf("Memory allocation failed\n");
		exit(1);
	}

	// If there was a previous allocation, copy the old pointers and free the old memory
	if (*tokens)
	{
		for (int i = 0; i < new_count - 1; i++)
		{ // Make sure you only copy existing tokens
			new_tokens[i] = (*tokens)[i];
		}
		free(*tokens);
	}
	else
	{
		for (int i = 0; i < new_count; i++)
		{
			new_tokens[i] = 0; // Initialise all new pointers to NULL
		}
	}

	// Redirect the passed tokens pointer to the new array
	*tokens = new_tokens;
}

char **tokenize(const char *input, int *token_count)
{
	char **tokens = 0; // Initialize the tokens array to 0
	const char *start = input;
	int length = 0;
	int result;
	*token_count = 0;
	int current_size = 0; // Variable to keep track of the current allocated size

	while (*input)
	{
		is_whitespace(*input, &result);
		if (result)
		{
			if (length > 0)
			{
				if (*token_count == current_size)
				{
					// Increase the size before allocating new token
					current_size += 1;
					allocate_tokens(&tokens, current_size);
				}
				malloc_token(tokens, *token_count, length);
				string_copy(tokens[*token_count], start, length);
				tokens[*token_count][length] = '\0';
				(*token_count)++;
				length = 0;
			}
		}
		else
		{
			if (length == 0)
			{
				start = input;
			}
			length++;
		}
		input++;
	}

	if (length > 0)
	{
		if (*token_count == current_size)
		{
			current_size += 1;
			allocate_tokens(&tokens, current_size);
		}
		malloc_token(tokens, *token_count, length);
		string_copy(tokens[*token_count], start, length);
		tokens[*token_count][length] = '\0';
		(*token_count)++;
	}

	print_tokens(tokens, *token_count);
	return tokens;
}

void free_tokens(char **tokens, int token_count)
{
	if (tokens)
	{
		for (int i = 0; i < token_count; i++)
		{
			free(tokens[i]);
			tokens[i] = 0;
		}
		free(tokens);
	}
}

void read_input(char *str)
{
	printf(">>> ");
	int bytesRead = read(0, str, INPUT_SIZE - 1);
	if (bytesRead > 0)
	{
		str[bytesRead - 1] = '\0';
	}

	printf("Input : [%s]\n", str);
}

void cd(char **tokens)
{
	// Error check token length in here?
	if (chdir(tokens[1]) < 0)
	{
		printf("cd: %s: No such file or directory.\n", tokens[1]);
	}
	else
	{
		printf("Moved into %s.\n", tokens[1]);
	}
}

void run(char **cmd)
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
}

int exit_shell(char *str)
{
	return (str[0] == 'e' && str[1] == 'x' && str[2] == 'i' && str[3] == 't' && str[4] == '\0');
}

int main(void)
{
	char input[INPUT_SIZE];
	while (1)
	{
		read_input(input);

		if (exit_shell(input))
		{
			printf("Exiting the shell...\n");
			break;
		}

		int token_count = 0;
		char **tokens = tokenize(input, &token_count);
		printf("tkn[0]: [%s]\n", tokens[0]);

		if ((token_count == 2) && (strcmp("cd", tokens[0]) == 0))
		{
			cd(tokens);
		}
		else
		{
			run(tokens);
		}

		//printf("\n\t-- FREEING --\n");
		free_tokens(tokens, token_count);
	}

	return 0;
}
