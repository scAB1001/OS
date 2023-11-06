#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h" // For the R,W,TRUNC etc

#define MAX_BUF_SZ 256
#define MAX_TOK_SZ MAX_BUF_SZ / 2 + 1

int is_whitespace(char c)
{
	return  (c == ' ' || c == '\t' || c == '\n');
	// XV6 might not support them... || c == '\v' || c == '\f' || c == '\r');
}

void clear_array(char **arr, int arrSize)
{
	if (arr)
	{
		for (int i = 0; i < arrSize; i++)
		{
			arr[i] = 0;
		}
	}
}

int count_strings(char **str)
{
    int count = 0;
    while (*str != 0)
    {
        count++;
        str++;
    }
    return count;
}



void print_tokens(char **tokens, int numTokens)
{
	printf("Tokens:");
	for (int i = 0; i < numTokens; i++)
	{
		printf(" [%s]", tokens[i]);
	}
	printf("\t There are %d tokens.\n", numTokens);
}

void tokenize(char *buf, char **tokens)
{
	int numTokens = 0, lenBuf = strlen(buf);
	tokens[0] = buf;
	for (int i = 0; i < lenBuf; i++)
	{
		while (is_whitespace(*tokens[i]))
		{
			tokens[i]++;
		}
		tokens[i + 1] = strchr(tokens[i], ' ');
		if (tokens[i + 1] == 0)
		{
			break;
		}
		*tokens[i + 1]++ = 0;
		numTokens++;
	}
	//tokens[numTokens] = 0;
	print_tokens(tokens, numTokens);
}


int read_input(char *str)
{
	printf(">>> ");
	memset(str, 0, MAX_BUF_SZ);
	if (gets(str, MAX_BUF_SZ) == 0) // EOF
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

void cd(char **tokens)
{
	if (chdir(tokens[1]) < 0)
	{
		printf("cd: %s: No such file or directory.\n", tokens[1]);
	}
	else
	{
		printf("Moved into %s\n", tokens[1]);
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
		exit(1);
	}
}/**/

int exit_shell(char *str)
{
	return strcmp(str, "exit") == 0;
}

int dup2(int oldfd, int newfd)
{
	if (oldfd == newfd)
		return newfd; // If the file descriptors are equal, nothing to do.

	close(newfd); // Close newfd if it's already open.

	// Duplicate oldfd to newfd using a loop, in case of interruption by a signal.
	int tempfd;
	while ((tempfd = dup(oldfd)) != newfd)
	{
		if (tempfd == -1)
			return -1; // If dup fails, return -1.

		if (tempfd > newfd)
		{
			close(tempfd); // If dup assigns a fd greater than newfd, try again.
		}
	}

	return newfd;
}

/*
void redirect_io(char *inFile, char *outFile) 
{
	// Handle input redirection
	if (inFile != 0)
	{
		close(0); // Close standard input

		if (open(inFile, O_RDONLY) < 0)
		{
			printf("Failed to open input file '%s'\n", inFile);
			exit(1);
		}
	}

	// Handle output redirection
	if (outFile != 0)
	{
		close(1); // Close standard output

		if (open(outFile, O_WRONLY | O_CREATE | O_TRUNC) < 0)
		{
			printf("Failed to open output file '%s'\n", outFile);
			exit(1);
		}
	}
}*/

void redirect_io(char *inFile, char *outFile) {
    int fd; // File descriptor

    // Handle input redirection
    if (inFile != NULL) {
        close(0); // Close standard input
        fd = open(inFile, O_RDONLY);
        if (fd < 0) {
            printf("Failed to open input file '%s'\n", inFile);
            exit(1);
        }
        if (fd != 0) {
            dup2(fd, 0);
            close(fd);
        }
    }

    // Handle output redirection
    if (outFile != NULL) {
        close(1); // Close standard output
        fd = open(outFile, O_WRONLY | O_CREAT | O_TRUNC, 0666); // Ensure the file is created if it does not exist
        if (fd < 0) {
            printf("Failed to open output file '%s'\n", outFile);
            exit(1);
        }
        if (fd != 1) {
            dup2(fd, 1);
            close(fd);
        }
    }
}


void part3(char **cmd)
{
    int i;
    char *inFile = 0, *outFile = 0;
    char *pipeCmd[MAX_BUF_SZ] = {0};
	
    // Parse the command to find redirection operators and the pipe operator
    for (i = 0; cmd[i] != 0; ++i)
    {
        if (strcmp(cmd[i], "<") == 0)
        {
            inFile = cmd[i + 1];
            cmd[i] = 0;
        }
        else if (strcmp(cmd[i], ">") == 0)
        {
            outFile = cmd[i + 1];
            cmd[i] = 0;
        }

        else if (strcmp(cmd[i], "|") == 0)
        {
            cmd[i] = 0;
            int j;

            // Collect the command after the pipe operator
            for (j = 0, i = i + 1; cmd[i] != 0; ++i, ++j)
            {
                pipeCmd[j] = cmd[i];
            }

            pipeCmd[j] = 0;
            break;
        }
    }

    // If a pipe is detected, set up the pipe and fork
    if (pipeCmd[0] != 0)
    {
        int p[2];

        if (pipe(p) < 0)
        {
            printf("Pipe creation failed\n");
            exit(1);
        }

        if (fork() == 0)
        {
            // Child process handles the command before the pipe
            close(p[0]);       // Close unused read end
            close(1);          // Close standard output
            dup(p[1]);         // Duplicate write end to standard output
            close(p[1]);       // Close original write end
			printf("\n\tHERE1\n");
            exec(cmd[0], cmd); // Execute the first part of the pipe command
            exit(0);
        }
        else
        {
            // Parent process handles the command after the pipe
            close(p[1]);               // Close unused write end
            close(0);                  // Close standard input
            dup(p[0]);                 // Duplicate read end to standard input
            close(p[0]);               // Close original read end
			printf("\n\tHERE2\n");
            exec(pipeCmd[0], pipeCmd); // Execute the second part of the pipe command
        }
    } 
    else
    {
		redirect_io(inFile, outFile);
		// Execute the command if there's no pipe
		printf("\n\tHERE3\n");
		exec(cmd[0], cmd);
    }
}

int main(void)
{
	static char input[MAX_BUF_SZ] = {0};
	char *tokens[MAX_TOK_SZ] = {0};

	while (read_input(input) == 0)
	{
		if (exit_shell(input))
		{
			printf("Exiting the shell...\n");
			break;
		}

		//clear_array(tokens, MAX_BUF_SZ);
		tokenize(input, tokens);

		printf("tkn[0]: [%s]\n", tokens[0]);
		if ((strcmp(tokens[0], "cd") == 0))
		{
			cd(tokens);
		}
		else if (count_strings(tokens) > 2)
		{
			part3(tokens);
		}
		else
		{
			run(tokens);
		}
		// printf("\n\t-- FREEING --\n");
	}

	return 0;
}

/**/