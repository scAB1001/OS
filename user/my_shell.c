#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

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

void realloc_tokens(char ***tokens, int new_size)
{
	// Reallocate the tokens array to hold 'new_size' number of pointers.
	char **temp = realloc(*tokens, new_size * sizeof(char *));
	if (!temp)
	{
		// If the reallocation fails, print an error message and exit.
		printf("Memory reallocation failed\n");
		exit(1);
	}
	// Update the tokens pointer to point to the newly allocated memory.
	*tokens = temp;
}

void malloc_token(char **tokens, int index, int length)
{
	// Allocate memory for the new token at the specified 'index',
	// including space for a null terminator.
	tokens[index] = malloc(length + 1);
	if (!tokens[index])
	{
		// If the memory allocation fails, print an error message and exit.
		printf("Memory allocation failed\n");
		exit(1);
	}
}

char **tokenize(const char *input, int *token_count)
{
	char **tokens = 0;
	const char *start = input; // Pointer to the start of a potential token
	int length = 0;			   // Length of the current token
	int result;				   // To hold the result from is_whitespace
	*token_count = 0;		   // Initialize the token count

	while (*input)
	{
		is_whitespace(*input, &result);
		if (result)
		{
			if (length > 0)
			{
				realloc_tokens(&tokens, *token_count + 1);
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
				start = input; // Update start to the current position
			}
			length++;
		}
		input++;
	}

	// Handle the last token if there is no trailing whitespace
	if (length > 0)
	{
		realloc_tokens(&tokens, *token_count + 1);
		malloc_token(tokens, *token_count, length);
		string_copy(tokens[*token_count], start, length);
		tokens[*token_count][length] = '\0';
		(*token_count)++;
	}

	print_tokens(tokens, *token_count);
	return tokens; // Return the array of tokens
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
	if (chdir(tokens[1]) < 0)
	{
		printf("cd: %s: No such file or directory.\n", tokens[1]);
	}
	else
	{
		printf("Please include a path.\n");
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
	/**/
}

int exit_shell(char *str)
{
	return str[0] == 'e' && str[1] == 'x' && str[2] == 'i' && str[3] == 't' && str[4] == '\0';
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

		printf("\n\t-- FREEING --\n");
		free_tokens(tokens, token_count);
	}

	return 0;
}
