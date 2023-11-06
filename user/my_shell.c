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

void execute_command(Command *cmd)
{
	char *argv[MAX_ARGS + 1]; // extra space for the program name and the null terminator
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

/* MULTI-PIPES WORK ONLY
int main(int argc, char *argv[])
{
	const int max_args = 32;
	const int max_commands = 8;
	const int command_buf_size = 128;
	char cmd_buf[command_buf_size];
	int num_commands = 0;
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
		num_commands = cmd_index + 1;

		if (num_commands > 0)
		{ // If there's at least one command, execute the pipeline
			execute_pipeline(commands, num_commands);
		}

		memset(commands, 0, sizeof(commands)); // Reset the commands array for the next input
		num_commands = 0;					   // Reset command count
	}

	exit(0);
}
*/

int main(int argc, char *argv[])
{
	char cmd_buf[COMMAND_BUF_SIZE];
	int num_commands = 0;
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

		// Check for "cd" command
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
		num_commands = cmd_index + 1;

		// Input/output redirection could be setup here before execute_pipeline is called
		// if needed, as per your shell's design.

		if (num_commands > 0)
		{ // If there's at least one command, execute the pipeline
			execute_pipeline(commands, num_commands);
		}

		memset(commands, 0, sizeof(commands)); // Reset the commands array for the next input
		num_commands = 0;					   // Reset command count
	}

	exit(0);
}
