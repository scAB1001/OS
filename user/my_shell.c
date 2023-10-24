#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void runCmd(char *cmd)
{
	int pid;

	// Fork a child process
	if ((pid = fork()) < 0)
	{
		fprintf(1, "Fork failed\n");
		return;
	}

	if (pid == 0)
	{
		// Child process
		exec(cmd, 0);
		printf("my_shell: %s not found\n", cmd);
		exit(0);
	}
	else
	{
		// Parent process
		wait(0); // Wait for the child process to finish
	}
}

int main(int argc, char *argv[]) {
	fprintf(1, "1");
	char input[128];

	while (1) {
		fprintf(0, ">>> ");

		// Using read system call to get input
		int bytesRead = read(0, input, sizeof(input));
		if (bytesRead <= 0)
		{
			break;
		}
		// Replace '\n' with '\0' to remove trailing newline
		input[bytesRead - 1] = '\0';

		//gets(input, sizeof(input));

		if (strcmp(input, "exit") == 0) {
			break; // Exit the shell
		}
		else if (strcmp(input, "ls") == 0)
		{
			// Handle "ls" command
			runCmd("ls");
		}
		// The space is to allow for a destination
		else if (input[0] == 'c' && input[1] == 'd' && input[2] == ' ')
		{
			char *directory = input + 3; // Extract the directory from the input
			if (chdir(directory) < 0)
			{
				fprintf(1, "cd: %s failed\n", directory);
			} 	
		}
		else
		{
			exit(5);
			/*// Execute the command using exec
			int pid = fork();
			if (pid == 0)
			{
				exec(input, 0);
				fprintf(1, "my_shell: %s not found\n", input);
				exit(1);
			}
			else
			{
				// Remove trailing newline character
				int len = strlen(input);
				if (input[len - 1] == '\n')
				{
					input[len - 1] = '\0';
				}
			}*/
		}
		}

	exit(0);
}


