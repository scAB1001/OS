#include "kernel/types.h"
#include "user/user.h"

int main()
{
        /* Usageint argc, char *argv[]
        if (argc < 2)
        {
                fprintf(2, "Usage: pingpong <byteToPing>\n");
                exit(0);
        }

        // 0. Temp byte to ping
        unsigned char *buf = (unsigned char *)argv[1];*/

        // 1. Create int array to pipe [2], pipe(arr_name); puts fd into arr
        char inputB = 'X';
        int p[2];

        // Check pipe() didn't fail
        if (pipe(p) < 0)
        {
                fprintf(2, "Pipe creation failed\n");
                close(p[0]);
                close(p[1]);
                exit(1);
        }
        
        // 2. Generate new process, giving child and parent processes
        int pid = fork();
        if (pid < 0)
        {
                fprintf(2, "Fork failed\n");
                close(p[0]);
                close(p[1]);
                exit(1);
        }

        // Check for the child process
        if (pid == 0)
        {
                // 2.
                char pingedB;
                read(p[0], &pingedB, 1);                            
                fprintf(1, "%d: received ping <%c>\n", getpid(), pingedB);

                if (pingedB == 'X')
                        pingedB = 'Q';
                
                write(p[1], &pingedB, 1);                                                  
                exit(0);
        }
        // Check for the parent process
        else
        {       // 1.
                write(p[1], &inputB, 1);
                pid = wait((int *)0);
                int my_pid = getpid();

                // --------------------------JUMP 2 CHILD PROCESS----------------------------- //

                // 3.
                char pongedB;
                read(p[0], &pongedB, 1);
                fprintf(1, "%d: received pong <%c>\n", my_pid, pongedB);
        }

        exit(0);
}
