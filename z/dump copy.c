
#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define MAX_BUF 128
#define MAX_ARGS 32

int is_whitespace(char c)
{
	return (c == ' ' || c == '\t' || c == '\n');
	// XV6 might not support them... || c == '\v' || c == '\f' || c == '\r');
}

int dup2(int oldfd, int newfd)
{
	if (oldfd == newfd)
	{
		return newfd;
	}

	close(newfd); // It's a good idea to check the return value here

	int tempfd;
	while ((tempfd = dup(oldfd)) != newfd)
	{ // Duplicate until we get the newfd
		if (tempfd == -1)
		{
			// Handle error: return -1 or exit
			return -1;
		}
		if (tempfd != newfd)
			close(tempfd); // Avoid leaking file descriptors
	}

	return newfd;
}

// Function to read a command line from input
int prompt_user(char *buf, int nbuf)
{
	printf(">>> "); // Print the prompt
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
		printf("Too many arguments\n");
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
		if (prompt_user(buf, sizeof(buf)) < 0)
			continue; // Skip if no command is entered

		tokenize(buf, argv, sizeof(argv) / sizeof(argv[0]), &argc);
		if (argc < 0)
			continue; // Skip if there was an error during tokenization

		// Check for "cd" command
		if (strcmp(argv[0], "cd") == 0)
		{
			if (argv[1] == 0)
			{
				printf("cd missing argument\n");
			}
			else
			{
				if (chdir(argv[1]) < 0)
				{
					printf("cd: failed to change directory to %s\n", argv[1]);
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
					exec(argv[0], argv);
					printf("exec %s failed\n", argv[0]);
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
					printf("exec %s failed\n", argv2[0]);
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