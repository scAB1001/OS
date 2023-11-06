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

/*
#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define MAX_ARGS 32
#define MAX_COMMANDS 8
#define COMMAND_BUF_SIZE 128

typedef struct
{
	char *args[MAX_ARGS]; // Command with arguments
} Command;

void printCommand(const Command *cmd)
{
	if (cmd == 0)
	{
		return; // Always good to check for 0 pointers
	}

	printf("\nTokens:");
	for (int i = 0; i < MAX_ARGS && cmd->args[i] != 0; i++)
	{
		printf(" [%s]", cmd->args[i]);
	}
	printf("\n");
}

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
int get_cmd(char *buf, int nbuf)
{
	fprintf(2, ">>> "); // Print the prompt
	memset(buf, 0, nbuf);
	gets(buf, nbuf);
	if (buf[0] == 0) // EOF
	{
		return -1;	 // Indicate end of file or no input
	}
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
		{
			break;
		}
		argv[argc++] = p;
		// Scan over arg
		while (!is_whitespace(*p) && *p != 0)
			p++;
		if (*p == 0)
		{
			break;
		}
		*p++ = 0; // 0 terminate and advance
	}

	if (argc >= argv_max)
	{
		fprintf(2, "Too many arguments\n");
		argc = -1; // Indicate error
	}
	else
	{
		argv[argc] = 0; // 0 terminate the argv list
	}

	if (argc_out != 0)
	{
		*argc_out = argc; // Pass back the number of arguments found
	}
}

void handle_redirection(Command *cmd)
{
	int fd;
	for (int i = 0; cmd->args[i] != 0; ++i)
	{
		if (strcmp(cmd->args[i], ">") == 0)
		{
			cmd->args[i] = 0; // Terminate args here
			if ((fd = open(cmd->args[i + 1], O_WRONLY | O_CREATE)) < 0)
			{
				fprintf(2, "open for writing error\n");
				exit(1);
			}
			dup2(fd, 1); // Redirect stdout to file
			close(fd);
			break;
		}
		else if (strcmp(cmd->args[i], "<") == 0)
		{
			cmd->args[i] = 0; // Terminate args here
			if ((fd = open(cmd->args[i + 1], O_RDONLY)) < 0)
			{
				fprintf(2, "open for reading error\n");
				exit(1);
			}
			dup2(fd, 0); // Redirect stdin from file
			close(fd);
			break;
		}
	}
}


void execute_command(Command *cmd)
{
	// First handle redirection if needed
	handle_redirection(cmd);

	// extra space for the program name and the 0 terminator
	char *argv[MAX_ARGS + 1];
	for (int i = 0; cmd->args[i] != 0; ++i)
	{
		argv[i] = cmd->args[i];
	}
	argv[MAX_ARGS] = 0; // 0-terminate the array

	if (exec(argv[0], argv) < 0)
	{
		fprintf(2, "exec failed\n");
		exit(1);
	}
}

void execute_pipeline(Command cmds[], int n)
{
	int fd[2];
	int pid;

	if (n == 0)
	{
		return; // No commands to execute
	}

	if (n == 1)
	{
		// Base case: execute the last command
		execute_command(&cmds[0]);
	}

	// Create a pipe
	if (pipe(fd) < 0)
	{
		fprintf(2, "pipe error\n");
		exit(1);
	}

	if ((pid = fork()) < 0)
	{
		fprintf(2, "fork error\n");
		exit(1);
	}

	if (pid == 0)
	{								// Child process
		close(fd[0]);				// Close read end
		dup2(fd[1], 1); // Connect write end of pipe to stdout
		close(fd[1]);				// Close original write end

		// Execute the first command
		execute_command(&cmds[0]);
	}
	else
	{							   // Parent process
		close(fd[1]);			   // Close write end
		dup2(fd[0], 0); // Connect read end of pipe to stdin
		close(fd[0]);			   // Close original read end

		// Recursively set up the rest of the pipeline
		execute_pipeline(cmds + 1, n - 1);

		wait((int *)0); // Wait for child process to finish
	}
}




int main(void)
{
	char cmd_buf[COMMAND_BUF_SIZE];
	int argc = 0;
	Command commands[MAX_COMMANDS]; // Array to store multiple commands

	while (get_cmd(cmd_buf, sizeof(cmd_buf)) >= 0)
	{ // Keep reading commands until EOF
		char *cmd_argv[MAX_ARGS];

		int cmd_argc;
		tokenize(cmd_buf, cmd_argv, MAX_ARGS, &cmd_argc); // Tokenize the input into arguments


		if (cmd_argc <= 0)
		{ // No command entered or error
			continue;
		}

		if (strcmp(cmd_argv[0], "q") == 0)
		{
			exit(0); // Exit the shell
		}

		if (strcmp(cmd_argv[0], "cd") == 0)
		{
			if (cmd_argc == 1)
			{
				fprintf(2, "cd missing argument\n");
			}
			else
			{
				if (chdir(cmd_argv[1]) < 0)
				{
					fprintf(2, "cd: failed to change directory to %s\n", cmd_argv[1]);
				}
			}
			continue; // Process next command
		}


		// Split the commands at each pipe character '|'
		int i = 0, j = 0, cmd_index = 0;
		commands[cmd_index].args[j++] = cmd_argv[i++];

		while (i < cmd_argc)
		{
			if (strcmp(cmd_argv[i], "|") == 0)
			{									 // Found a pipe, switch to the next command
				commands[cmd_index].args[j] = 0; // Terminate current command arguments
				cmd_index++;
				j = 0;
				if (cmd_index >= MAX_COMMANDS)
				{
					fprintf(2, "Too many commands\n");
					exit(1);
				}
				i++; // Skip over the pipe token
			}
			else
			{ // Otherwise, keep adding to the current command
				commands[cmd_index].args[j++] = cmd_argv[i++];
			}
		}
		commands[cmd_index].args[j] = 0; // Terminate last command arguments
		argc = cmd_index + 1;

		printCommand(commands);

		// Input/output redirection could be setup here before execute_pipeline is called
		// if needed, as per your shell's design.

		if (argc == 1)
		{
			handle_redirection(&commands[0]); // Handle input/output redirection
			execute_command(&commands[0]);
		}
		else
		{ // If there are pipes, set up the pipeline
			execute_pipeline(commands, argc);
		}

		memset(commands, 0, sizeof(commands)); // Reset the commands array for the next input
		argc = 0;					   // Reset command count
	}
	exit(0);
}

/* MULTI-PIPES WORK ONLY NO I/O WORKS?? -- REVISIT
int main(int argc, char *argv[])
{
	const int max_args = 32;
	const int max_commands = 8;
	const int command_buf_size = 128;
	char cmd_buf[command_buf_size];
	int argc = 0;
	Command commands[max_commands]; // Array to store multiple commands

	while (get_cmd(cmd_buf, sizeof(cmd_buf)) >= 0)
	{ // Keep reading commands until EOF
		char *cmd_argv[max_args];
		int cmd_argc;

		tokenize(cmd_buf, cmd_argv, max_args, &cmd_argc); // Tokenize the input into arguments

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
		}

		if (cmd_argc <= 0)
		{ // No command entered or error
			continue;
		}

		// Split the commands at each pipe character '|'
		int i = 0, j = 0, cmd_index = 0;
		commands[cmd_index].args[j++] = cmd_argv[i++];

		while (i < cmd_argc)
		{
			if (strcmp(cmd_argv[i], "|") == 0)
			{										// Found a pipe, switch to the next command
				commands[cmd_index].args[j] = 0; // Terminate current command arguments
				cmd_index++;
				j = 0;
				if (cmd_index >= max_commands)
				{
					fprintf(2, "Too many commands\n");
					exit(1);
				}
			}
			else
			{ // Otherwise, keep adding to the current command
				commands[cmd_index].args[j++] = cmd_argv[i];
			}
			i++;
		}
		commands[cmd_index].args[j] = 0; // Terminate last command arguments
		argc = cmd_index + 1;

		if (argc > 0)
		{ // If there's at least one command, execute the pipeline
			execute_pipeline(commands, argc);
		}

		memset(commands, 0, sizeof(commands)); // Reset the commands array for the next input
		argc = 0;					   // Reset command count
	}

	exit(0);
}
*/

*/