#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main()
{
        int parentToChild[2], childToParent[2];
        char buf;

        if (pipe(parentToChild) < 0 || pipe(childToParent) < 0)
        {  // Usage check
                fprintf(2, "Pipe creation failed\n");
                exit(1);
        }

        int pid = fork();

        if (pid < 0)
        {
                fprintf(2, "Fork failed\n");
                exit(1);
        }

        if (pid == 0)
        {  // Child process
                close(parentToChild[1]);                                // Close write end of parentToChild pipe
                close(childToParent[0]);                                // Close read end of childToParent pipe

                read(parentToChild[0], &buf, sizeof(buf));              // Read a byte from parent
                fprintf(1, "%d: received ping\n", getpid());

                write(childToParent[1], &buf, sizeof(buf));             // Write the byte to parent

                close(parentToChild[0]);                                // Close read end of parentToChild pipe
                close(childToParent[1]);                                // Close write end of childToParent pipe
                exit(0);
        }
        else
        {  // Parent process
                close(parentToChild[0]);                                // Close read end of parentToChild pipe
                close(childToParent[1]);                                // Close write end of childToParent pipe

                char message[] = "X";
                write(parentToChild[1], message, sizeof(message));      // Send a byte to child

                read(childToParent[0], &buf, sizeof(buf));              // Read a byte from child
                fprintf(1, "%d: received pong\n", getpid());

                close(parentToChild[1]);                                // Close write end of parentToChild pipe
                close(childToParent[0]);                                // Close read end of childToParent pipe
                exit(0);
        }

        exit(0);
}
