#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"


#define MAX_BUF_SZ_BUF_SZ 256


void is_whitespace(char c, int *result)
{
	*result = (c == ' ' || c == '\t' || c == '\n');
	// XV6 might not support them... || c == '\v' || c == '\f' || c == '\r');
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
				strcpy(tokens[*token_count], start);
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
		strcpy(tokens[*token_count], start);
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
		tokens = 0;
	}
}

int read_input(char *str)
{
	printf(">>> ");
	memset(str, 0, MAX_BUF_SZ_BUF_SZ);
	gets(str, MAX_BUF_SZ_BUF_SZ);

	if (str[0] == 0) // EOF
	{
		return -1;
	}

	// Remove newline character if present
    int len = strlen(str);
    if(len > 0 && str[len-1] == '\n') 
	{
        str[len-1] = '\0';
    }

	printf("Input : [%s]\t Len: %d\n", str, strlen(str));
	return 0;
}

void cd(char *str)
{
	if (chdir(str+3) < 0)
	{
		printf("cd: %s: No such file or directory.\n", str+3);
	}
	else
	{
		printf("Moved into %s\n", str+3);
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
	char input[MAX_BUF_SZ_BUF_SZ];
	while (read_input(input) >= 0)
	{
		if ((strlen(input) == 4) && (strcmp(input, "exit") == 0))
		{
			printf("Exiting the shell...\n");
			break;
		}
		if ((strlen(input) == 3) && (strcmp(input, "cd ") == 0))
		{
			cd(input+3);
		}

		int token_count = 0;
		char **tokens = tokenize(input, &token_count);
		printf("tkn[0]: [%s]\n", tokens[0]);

		run(tokens);

		//printf("\n\t-- FREEING --\n");
		free_tokens(tokens, token_count);
	}

	return 0;
}

/*
int getcmd(char *buf, int nbuf) {
    printf(">>>");
    memset(buf, 0, nbuf);
    gets(buf, nbuf);
    if(buf[0] == 0) // EOF
        return -1;
    return 0;
}

void runcmd(char *cmd) {
    if (cmd == 0)
        exit(0);

    if (strcmp(cmd, "cd ") == 0) {
        // 'cd' command is not executed via fork and exec because it changes the current
        // directory for the shell process, not a child process.
        if(chdir(cmd+3) < 0)
            printf("cannot cd %s\n", cmd+3);
        return;
    }

    char *argv[MAX_BUF_SZ];
    int argc = 0;
    argv[argc] = cmd;
    while(*cmd != '\0') {
        if (*cmd == ' ') {
            *cmd++ = '\0';
            while (*cmd == ' ') cmd++;
            if (*cmd != '\0')
                argv[++argc] = cmd;
        } else {
            cmd++;
        }
    }
    argv[++argc] = 0;

    int pid = fork();
    if (pid == 0) {
        exec(argv[0], argv);
        printf("exec %s failed\n", argv[0]);
    } else if (pid > 0) {
        wait(0);
    } else {
        printf("fork failed\n");
    }
}

int main(void) {
    static char buf[MAX_BUF_SZ];

    while (getcmd(buf, sizeof(buf)) >= 0) {
        runcmd(buf);
    }
    exit(0);
}*/
