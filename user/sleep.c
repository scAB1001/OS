#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int main(int argc, char *argv[]) 
{
	if(argc < 2)
	{
		fprintf(2, "Usage: sleep X(seconds)...\n");
		exit(1);
	}

	sleep(10*atoi(argv[1]));
	for(int i=0; i < 5; i++)
	{
		printf("WAKEUPWAKEUPWAKEUP\n");
	}
	exit(0);
};
