#include "ult.h"
#include "array.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define _XOPEN_SOURCE
#include <ucontext.h>

/* thread control block */
typedef struct tcb_s
{
	/* data needed to restore the context */
} tcb_t;

void ult_init(ult_f f)
{}

int ult_spawn(ult_f f)
{	
	return 0;		
}

void ult_yield()
{}

void ult_exit(int status)
{}

int ult_join(int tid, int* status)
{
	return -1;
}

ssize_t ult_read(int fd, void* buf, size_t size)
{
	return 0;
}
