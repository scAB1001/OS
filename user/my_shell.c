

/*
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
}

int exit_shell(char *str)
{
	return strcmp(str, "exit") == 0;
}

int dup2(int oldfd, int newfd)
{
	if (oldfd == newfd)
	{
		return newfd; // If the file descriptors are the same, nothing to do
	}
	close(newfd); // Close the new file descriptor if it's already open

	return dup(oldfd); // Duplicate the old file descriptor to new
}

void redirect_io(char *inFile, char *outFile) {
    int fd; // File descriptor

    // Handle input redirection
    if (inFile != 0) 
	{
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
    if (outFile != 0) 
	{
        close(1); // Close standard output
        fd = open(outFile, O_WRONLY | O_CREATE | O_TRUNC); // Ensure the file is created if it does not exist
        if (fd < 0) 
		{
            printf("Failed to open output file '%s'\n", outFile);
            exit(1);
        }
        if (fd != 1) 
		{
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
	static char buf[MAX_BUF_SZ] = {0};
	char *tokens[MAX_TOK_SZ] = {0};
	int fd_pipe[2];

	while (read_input(buf) == 0)
	{
		if (exit_shell(buf))
		{
			printf("Exiting the shell...\n");
			break;
		}

		//clear_array(tokens, MAX_BUF_SZ);
		tokenize(buf, tokens);

		//printf("tkn[0]: [%s]\n", tokens[0]);
		if ((strcmp(tokens[0], "cd") == 0))
		{
			cd(tokens);
		}

		// Initialize file descriptors for redirection
		int redirect_in = -1, redirect_out = -1;

		// Check for redirection
		for (int i = 0; tokens[i]; i++)
		{
			if (strcmp(tokens[i], ">") == 0)
			{
				// Output redirection
				tokens[i] = 0; // Terminate the arguments for exec
				if (tokens[i + 1])
				{
					redirect_out = open(tokens[i + 1], O_WRONLY | O_CREATE);
					if (redirect_out < 0)
					{
						printf("Cannot open file %s\n", tokens[i + 1]);
						exit(1);
					}
				}
				else
				{
					printf("Output redirection requires a filename\n");
					exit(1);
				}
				i++; // Skip the filename
			}
			else if (strcmp(tokens[i], "<") == 0)
			{
				// Input redirection
				tokens[i] = 0; // Terminate the arguments for exec
				if (tokens[i + 1])
				{
					redirect_in = open(tokens[i + 1], O_RDONLY);
					if (redirect_in < 0)
					{
						printf("Cannot open file %s\n", tokens[i + 1]);
						exit(1);
					}
				}
				else
				{
					printf("Input redirection requires a filename\n");
					exit(1);
				}
				i++; // Skip the filename
			}
		}

		// Look for the pipe symbol
		int i;
		int piped = 0; // A flag to check if we have found a pipe
		for (i = 0; tokens[i]; i++)
		{
			if (strcmp(tokens[i], "|") == 0)
			{								 // If pipe is found
				piped = 1;					 // Set piped flag
				tokens[i] = 0;				 // Null-terminate the first command
				char **tokens2 = &tokens[i + 1]; // Get the second command's arguments

				if (pipe(fd_pipe) < 0)
				{
					printf("pipe failed\n");
					exit(1);
				}

				int pid1 = fork();
				if (pid1 == 0)
				{
					// First child: executes the first command
					close(fd_pipe[0]);	 // Close unused read end
					dup2(fd_pipe[1], 1); // Redirect stdout to pipe write
					close(fd_pipe[1]);	 // Close pipe write, not required anymore
					exec(tokens[0], tokens);
					printf("exec %s failed\n", tokens[0]);
					exit(1);
				}

				int pid2 = fork();
				if (pid2 == 0)
				{
					// Second child: executes the second command
					close(fd_pipe[1]);	 // Close unused write end
					dup2(fd_pipe[0], 0); // Redirect stdin to pipe read
					close(fd_pipe[0]);	 // Close pipe read, not required anymore
					exec(tokens2[0], tokens2);
					printf("exec %s failed\n", tokens2[0]);
					exit(1);
				}

				// Parent closes both ends of the pipe and waits for children
				close(fd_pipe[0]);
				close(fd_pipe[1]);
				wait(0);
				wait(0);

				// Once the pipe handling is done, break out of the loop to wait for the next command
				break;
			}
		}

		// If no pipe was found, then execute a single command
		if (!piped)
		{
			int pid = fork();
			if (pid == 0)
			{
				// Child process
				if (redirect_in != -1)
				{
					dup2(redirect_in, 0); // Replace stdin with input file
					close(redirect_in);	  // Close original file descriptor
				}
				if (redirect_out != -1)
				{
					dup2(redirect_out, 1); // Replace stdout with output file
					close(redirect_out);   // Close original file descriptor
				}
				exec(tokens[0], tokens);
				printf("exec %s failed\n", tokens[0]);
				exit(1);
			}
			else if (pid > 0)
			{
				// Parent process
				wait(0);
				if (redirect_in != -1)
					close(redirect_in); // Close file descriptor if used
				if (redirect_out != -1)
					close(redirect_out); // Close file descriptor if used
			}
			else
			{
				printf("fork failed\n");
				exit(1);
			}
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
*/

/**/
#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define MAX_BUF 100
#define MAX_ARGS 10

int is_whitespace(char c)
{
	return (c == ' ' || c == '\t' || c == '\n');
	// XV6 might not support them... || c == '\v' || c == '\f' || c == '\r');
}

int dup2(int oldfd, int newfd)
{
	if (oldfd == newfd)
		return newfd;  // If the file descriptors are the same, nothing to do
	close(newfd);	   // Close the new file descriptor if it's already open
	return dup(oldfd); // Duplicate the old file descriptor to new
}

// Function to read a command line from input
int get_cmd(char *buf, int nbuf)
{
	fprintf(2, ">>> "); // Print the prompt
	memset(buf, 0, nbuf);
	gets(buf, nbuf);
	if (buf[0] == 0) // EOF
		return -1;	 // Indicate end of file or no input
	return 0;		 // Indicate success
}

// Function to split command line into arguments
void tokenize(char *buf, char *argv[], int argv_max, int *argc_out)
{
	char *p = buf;
	int argc = 0;
	while (argc < argv_max)
	{
		// Skip leading spaces
		while (is_whitespace(*p))
			p++;
		if (*p == 0)
			break;
		argv[argc++] = p;
		// Scan over arg
		while (!is_whitespace(*p) && *p != 0)
			p++;
		if (*p == 0)
			break;
		*p++ = 0; // Null terminate and advance
	}

	if (argc >= argv_max)
	{
		fprintf(2, "Too many arguments\n");
		argc = -1; // Indicate error
	}
	else
	{
		argv[argc] = 0; // Null terminate the argv list
	}

	if (argc_out != 0)
	{
		*argc_out = argc; // Pass back the number of arguments found
	}
}

int main(void)
{
	static char buf[MAX_BUF];
	char *argv[MAX_ARGS];
	int fd_pipe[2];
	int argc;

	while (1)
	{
		if (get_cmd(buf, sizeof(buf)) < 0)
			continue; // Skip if no command is entered

		tokenize(buf, argv, sizeof(argv) / sizeof(argv[0]), &argc);
		if (argc < 0)
			continue; // Skip if there was an error during tokenization

		// Check for "cd" command
		if (strcmp(argv[0], "cd") == 0)
		{
			if (argv[1] == 0)
			{
				fprintf(2, "cd missing argument\n");
			}
			else
			{
				if (chdir(argv[1]) < 0)
				{
					fprintf(2, "cd: failed to change directory to %s\n", argv[1]);
				}
			}
			continue; // "cd" is handled in the shell process
		}

		// Initialize file descriptors for redirection
		int redirect_in = -1, redirect_out = -1;

		// Check for redirection
		for (int i = 0; argv[i]; i++)
		{
			if (strcmp(argv[i], ">") == 0)
			{
				// Output redirection
				argv[i] = 0; // Terminate the arguments for exec
				if (argv[i + 1])
				{
					redirect_out = open(argv[i + 1], O_WRONLY | O_CREATE);
					if (redirect_out < 0)
					{
						printf("Cannot open file %s\n", argv[i + 1]);
						exit(1);
					}
				}
				else
				{
					printf("Output redirection requires a filename\n");
					exit(1);
				}
				i++; // Skip the filename
			}
			else if (strcmp(argv[i], "<") == 0)
			{
				// Input redirection
				argv[i] = 0; // Terminate the arguments for exec
				if (argv[i + 1])
				{
					redirect_in = open(argv[i + 1], O_RDONLY);
					if (redirect_in < 0)
					{
						printf("Cannot open file %s\n", argv[i + 1]);
						exit(1);
					}
				}
				else
				{
					printf("Input redirection requires a filename\n");
					exit(1);
				}
				i++; // Skip the filename
			}
		}

		// Look for the pipe symbol
		int i;
		int piped = 0; // A flag to check if we have found a pipe
		for (i = 0; argv[i]; i++)
		{
			if (strcmp(argv[i], "|") == 0)
			{								 // If pipe is found
				piped = 1;					 // Set piped flag
				argv[i] = 0;				 // Null-terminate the first command
				char **argv2 = &argv[i + 1]; // Get the second command's arguments

				if (pipe(fd_pipe) < 0)
				{
					fprintf(2, "pipe failed\n");
					exit(1);
				}

				int pid1 = fork();
				if (pid1 == 0)
				{
					// First child: executes the first command
					close(fd_pipe[0]);	 // Close unused read end
					dup2(fd_pipe[1], 1); // Redirect stdout to pipe write
					close(fd_pipe[1]);	 // Close pipe write, not required anymore
					exec(argv[0], argv);
					fprintf(2, "exec %s failed\n", argv[0]);
					exit(1);
				}

				int pid2 = fork();
				if (pid2 == 0)
				{
					// Second child: executes the second command
					close(fd_pipe[1]);	 // Close unused write end
					dup2(fd_pipe[0], 0); // Redirect stdin to pipe read
					close(fd_pipe[0]);	 // Close pipe read, not required anymore
					exec(argv2[0], argv2);
					fprintf(2, "exec %s failed\n", argv2[0]);
					exit(1);
				}

				// Parent closes both ends of the pipe and waits for children
				close(fd_pipe[0]);
				close(fd_pipe[1]);
				wait(0);
				wait(0);

				// Once the pipe handling is done, break out of the loop to wait for the next command
				break;
			}
		}

		// If no pipe was found, then execute a single command
		if (!piped)
		{
			int pid = fork();
			if (pid == 0)
			{
				// Child process// Handle redirection and pipes
				void handle_redirection_and_pipes(char *argv[])
				{
					int redirect_in = -1, redirect_out = -1, fd_pipe[2];
					int piped = 0;
					if (redirect_in != -1)
					{
						dup2(redirect_in, 0); // Replace stdin with input file
						close(redirect_in);	  // Close original file descriptor
					}
					if (redirect_out != -1)
					{
						dup2(redirect_out, 1); // Replace stdout with output file
						close(redirect_out);   // Close original file descriptor
					}
					exec(argv[0], argv);
					printf("exec %s failed\n", argv[0]);
					exit(1);
				}
			else if (pid > 0)
			{
				// Parent process
				wait(0);
				if (redirect_in != -1)
					close(redirect_in); // Close file descriptor if used
				if (redirect_out != -1)
					close(redirect_out); // Close file descriptor if used
			}
			else
			{
				printf("fork failed\n");
				exit(1);
			}
		}
	}
	exit(0);
}