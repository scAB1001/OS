#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define MAX_ARGS 32			 // Maximum number of arguments
#define MAX_COMMANDS 8		 // Maximum number of commands for multi-command processing (unused here)
#define COMMAND_BUF_SIZE 128 // Buffer size for command input

// Structure representing a single command with its arguments
typedef struct
{
	// An array of strings to hold the arguments of the command
	char *args[MAX_ARGS];
} Command;

// Function to print the command arguments to the console for debugging
void printCommand(const Command *cmd)
{
	if (cmd == 0)
	{ // If cmd is a null ptr, return to prevent dereferencing null
		return;
	}

	printf("\nTokens:");
	for (int i = 0; i < MAX_ARGS && cmd->args[i] != 0; i++)
	{
		printf(" [%s]", cmd->args[i]);
	}
	printf("\n");
}

// Function to check if a character is whitespace
int is_whitespace(char c)
{
	// A character is whitespace if it is a space, tab, or newline
	return (c == ' ' || c == '\t' || c == '\n');
	// Additional whitespace characters are not supported in XV6
}

// Function to duplicate a Fd
int dup2(int oldfd, int newfd)
{
	if (oldfd == newfd)
	{ // If the Fds are the same, no action is needed
		return newfd;
	}
	// Close newfd if open; ideally should check the return value
	close(newfd);

	int tempfd;
	while ((tempfd = dup(oldfd)) != newfd)
	{
		if (tempfd == -1)
		{ // If an error occurred during dup, return -1
			return -1;
		}
		if (tempfd != newfd)
		{ // Close the temporary Fd to avoid leaks
			close(tempfd);
		}
	}

	return newfd;
}

// Function to run a command using fork and exec
void run(char **str)
{
	int pid;

	if ((pid = fork()) < 0)
	{
		printf("Fork failed\n");
		return;
	}

	if (pid == 0)
	{
		// In the child process
		if (exec(str[0], str) < 0)
		{
			printf("exec %s failed\n", *str);
			exit(1);
		}
	}
	else
	{ // Parent process waits for the child to complete
		wait(0);
	}
}

// Function to read a command from the standard input
int prompt(char *buf, int nbuf)
{
	fprintf(2, ">>> ");	  // Print a command prompt to stderr
	memset(buf, 0, nbuf); // Clear the buffer
	gets(buf, nbuf);	  // Read input from the user into the buffer
	if (buf[0] == 0)
		return -1; // If the buffer is empty, return -1 to indicate EOF or no input
	return 0;	   // Return 0 to indicate success
}

// Function to split command line into arguments
// The function takes a buffer string containing the command line input (buf),
// an array to store the ptrs to arguments (argv), the maximum number of
// arguments to parse (argv_max), and a ptr to an integer to store the number
// of arguments parsed (argc_out).

// Using a State Machine to tokenize an input string
void tokenize(char *buf, char **argv, int argv_max, int *argc_out)
{
	// Define a finite state machine with two states indicating whether
	// we're currently scanning inside a token or outside of one.
	enum
	{
		OUTSIDE,
		INSIDE
	} state = OUTSIDE;

	// Counter for how many tokens have been found.
	int argc = 0;

	// Iterate through the buffer character by character until
	// the end of the string is reached or the token count is reached.
	while (*buf && argc < argv_max)
	{
		// Check if the current character is whitespace.
		if (is_whitespace(*buf))
		{
			// If we're inside a token and encounter whitespace, end the token here.
			if (state == INSIDE)
			{
				*buf = '\0'; // Null-terminate the current token.
			}
			// After handling the token (if any), transition to OUTSIDE state.
			state = OUTSIDE;
		}
		else // The current character is not a whitespace, so it's part of a token.
		{
			// If we're outside a token and find a non-whitespace character,
			// it marks the beginning of a new token.
			if (state == OUTSIDE)
			{ // Set the start of the new token and increment the token count.
				argv[argc++] = buf;
			}
			// Transition to INSIDE state since we're now inside a token.
			state = INSIDE;
		}
		// Move to the next character in the buffer.
		buf++;
	}

	// Null-terminate the argv array if we haven't filled it to the maximum.
	// This check is important in case the buffer ends exactly on argv_max - 1 tokens.
	if (argc < argv_max)
	{
		argv[argc] = 0;
	}

	// If the argument count has reached or exceeded the maximum limit,
	// indicate an error and inform the user
	if (argc >= argv_max)
	{
		fprintf(2, "Too many arguments\n");
		argc = -1; // Indicate error
	}

	// If a ptr for the output argument count was provided,
	// set it to the number of arguments we've parsed
	if (argc_out)
	{
		*argc_out = argc;
	}
}

// Function to handle input and output redirection in a command.
// It takes a ptr to a Command structure which contains the command's arguments.
void handle_redirection(Command *cmd)
{
	printf("\nHERE 1\n");
	int fd; // Fd for the file to be opened for redirection

	// Iterate over the command's arguments to find redirection symbols
	for (int i = 0; cmd->args[i] != 0; ++i)
	{
		// If the '>' symbol is found, set up output redirection
		if (strcmp(cmd->args[i], ">") == 0)
		{
			cmd->args[i] = 0; // Terminate the arguments array at the redirection symbol
			// Attempt to open the file for writing
			if ((fd = open(cmd->args[i + 1], O_WRONLY | O_CREATE)) < 0)
			{
				fprintf(2, "open for writing error\n"); // Report an error if the file cannot be opened
				exit(1);
			}
			dup2(fd, 1); // Redirect standard output (Fd 1) to the file
			close(fd);	 // Close the Fd as it's no longer needed
			break;		 // Stop processing further as we've handled the redirection
		}
		// If the '<' symbol is found, set up input redirection
		else if (strcmp(cmd->args[i], "<") == 0)
		{
			cmd->args[i] = 0; // Terminate the arguments array at the redirection symbol
			// Attempt to open the file for reading
			if ((fd = open(cmd->args[i + 1], O_RDONLY)) < 0)
			{
				fprintf(2, "open for reading error\n"); // Report an error if the file cannot be opened
				exit(1);
			}
			dup2(fd, 0); // Redirect standard input (Fd 0) from the file
			close(fd);	 // Close the Fd as it's no longer needed
			break;		 // Stop processing further as we've handled the redirection
		}
	}
}

void execute_command(Command *cmd)
{
	// If redirection is necessary (i.e., changing the standard input/output), handle it first
	handle_redirection(cmd);

	// Additional space is allocated for the program name and a NULL terminator
	char *argv[MAX_ARGS + 1];
	// Copy pointers from cmd->args to argv, making sure to preserve NULL termination
	for (int i = 0; cmd->args[i] != 0; ++i)
	{
		argv[i] = cmd->args[i];
	}
	argv[MAX_ARGS] = 0; // Ensure the argument list is NULL terminated

	// Execute the command and check for errors
	if (exec(argv[0], argv) < 0)
	{
		// If exec returns, it must have failed, print an error and exit
		fprintf(2, "exec failed\n");
		exit(1);
	}
}

// Function to count the number of pipes in a command array
int count_pipes(char **argv)
{
	int count = 0;
	while (*argv)
	{
		if (strcmp(*argv, "|") == 0)
		{
			count++;
		}

		argv++;
	}

	return count;
}

// Function to redirect the output to the pipe
void redirect_output(int *fds)
{
	close(fds[0]);	 // Close the read end of the pipe
	dup2(fds[1], 1); // Duplicate the write end of the pipe to stdout
	close(fds[1]);	 // Close the original write end as it is no longer needed
}

// Function to redirect the input from the pipe
void redirect_input(int *fds)
{
	close(fds[1]);	 // Close the write end of the pipe
	dup2(fds[0], 0); // Duplicate the read end of the pipe to stdin
	close(fds[0]);	 // Close the original read end as it is no longer needed
}

// Function to execute a single pipe command
void exec_pipe_command(char **argv)
{
	int fd_pipe[2];
	if (pipe(fd_pipe) < 0)
	{
		printf("pipe fail\n");
		exit(1);
	}

	int pid = fork();
	if (pid < 0)
	{
		printf("fork fail\n");
		exit(1);
	}

	else if (pid == 0)
	{
		// First command (write to the pipe)
		redirect_output(fd_pipe);

		// Find the pipe character and execute the first command
		for (int i = 0; argv[i] != 0; i++)
		{
			if (strcmp(argv[i], "|") == 0)
			{
				argv[i] = 0;

				// Execute the command and check for errors
				if (exec(argv[0], argv) < 0)
				{
					// If exec returns, it must have failed, print an error and exit
					fprintf(2, "exec failed\n");
					exit(1);
				}

				exit(1);
			}
		}
	}
	else
	{
		// Second command (read from the pipe)
		redirect_input(fd_pipe);

		// Wait for the first command to finish
		wait(0);

		// Execute the second command
		char **second_command = 0;
		for (int i = 0; argv[i] != 0; i++)
		{
			if (second_command != 0)
			{
				second_command[i] = argv[i];
			}
			if (strcmp(argv[i], "|") == 0)
			{
				second_command = &argv[i + 1];
			}
		}

		// Execute the command and check for errors

		if (exec(second_command[0], second_command) < 0)
		{
			// If exec returns, it must have failed, print an error and exit
			fprintf(2, "exec failed\n");
			exit(1);
		}

		exit(1);
	}
}

void execute_pipeline(Command *cmds, int n)
{
	int fd[2]; // File descriptors for the pipe
	int pid;   // Process ID from fork()

	if (n == 0)
	{
		return; // If there are no commands, there's nothing to execute
	}

	if (n == 1)
	{
		// If there's only one command left, execute it without setting up a pipeline
		execute_command(&cmds[0]);
	}

	// Create a pipe and check for errors
	if (pipe(fd) < 0)
	{
		fprintf(2, "pipe error\n");
		exit(1);
	}

	// Fork the current process and check for errors
	if ((pid = fork()) < 0)
	{
		fprintf(2, "fork error\n");
		exit(1);
	}

	if (pid == 0)
	{	// Child process branch
		redirect_output(fd);

		// Execute the first command in the pipeline
		execute_command(&cmds[0]);
	}
	else
	{	// Parent process branch
		redirect_input(fd);

		// Recursively continue setting up the pipeline for the remaining commands
		execute_pipeline(cmds + 1, n - 1);

		// Wait for the child process to finish before continuing
		wait((int *)0);
	}
}

int main(void)
{
	char cmd_buf[COMMAND_BUF_SIZE]; // Buffer to hold the command line input
	int argc = 0;					// Argument count
	Command commands[MAX_COMMANDS]; // Array to store commands for a pipeline

	// Loop to read commands until EOF (End Of File) is encountered
	while (prompt(cmd_buf, sizeof(cmd_buf)) >= 0)
	{
		// Array to store the arguments of a single command
		char *cmd_argv[MAX_ARGS];

		int cmd_argc; // Store the count of arguments for the current command
		// Tokenize the input buffer into command arguments
		tokenize(cmd_buf, cmd_argv, MAX_ARGS, &cmd_argc);

		// If no arguments are parsed or if an error occurs, skip the current loop iteration
		if (cmd_argc <= 0)
		{
			continue;
		}

		// If the first argument is 'q', then exit the shell program
		if (strcmp(cmd_argv[0], "q") == 0)
		{
			exit(0);
		}

		// Change directory command 'cd' with error handling for argument count
		if (strcmp(cmd_argv[0], "cd") == 0)
		{
			if (cmd_argc == 1) // If 'cd' is issued without any arguments, report an error
			{
				fprintf(2, "cd missing argument\n");
			}
			else // Attempt to change the directory and report an error if unsuccessful
			{
				if (chdir(cmd_argv[1]) < 0)
				{
					fprintf(2, "cd: failed to change directory to %s\n", cmd_argv[1]);
				}
			}
			continue; // Move on to the next command input
		}

		// Parsing for handling a series of piped commands (separated by '|')
		int i = 0, j = 0, cmd_index = 0;
		// Start by assigning the first argument to the first command
		commands[cmd_index].args[j++] = cmd_argv[i++];

		// Loop over all arguments to split commands at each pipe character '|'
		while (i < cmd_argc)
		{
			if (strcmp(cmd_argv[i], "|") == 0)
			{									 // If a pipe is encountered
				commands[cmd_index].args[j] = 0; // Null-terminate the current command's arguments
				cmd_index++;					 // Move to the next command
				j = 0;							 // Reset argument index for the new command
				// Check if the maximum number of commands has been reached
				if (cmd_index >= MAX_COMMANDS)
				{
					fprintf(2, "Too many commands\n");
					exit(1);
				}
				i++; // Skip the pipe symbol
			}
			else
			{ // Add the current argument to the current command
				commands[cmd_index].args[j++] = cmd_argv[i++];
			}
		}
		commands[cmd_index].args[j] = 0; // Null-terminate the last command's arguments
		argc = cmd_index + 1;			 // Calculate the number of commands to execute

		// Execute the pipeline of commands if any command(s) exist
		if (argc > 0)
		{
			execute_pipeline(commands, argc);
		}

		// Clear the commands array for the next round of input
		memset(commands, 0, sizeof(commands));
		// Reset the command count for the next iteration
		argc = 0;
	}

	// Exit the shell if the loop ends (e.g., due to an EOF on input)
	exit(0);
}