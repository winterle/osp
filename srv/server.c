#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PORT 9000

int main()
{
	while (1)
	{
		printf("[srv]: idle\n");
		sleep(1);
	}
	
	return 0;
}
