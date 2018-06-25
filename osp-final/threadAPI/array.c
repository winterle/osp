/***************************************************************************//**
 * @file array.c
 * @author Dorian Weber
 * @brief Implementation des generalisierten Arrays.
 ******************************************************************************/

#include "array.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* ********************************************************* public functions */

/* (die runden Klammern um einige Funktionsnamen sind notwendig, da Makros
 * gleichen Namens existieren und der Präprozessor diese expandieren würde) */

void* (arrayInit)(unsigned int capacity, size_t size)
{
	array_hdr_t* hdr = malloc(sizeof(*hdr) + size*capacity);
	
	if (hdr == NULL)
		return NULL;
	
	hdr->len = 0;
	hdr->cap = capacity;
	
	return hdr + 1;
}

void arrayRelease(void* self)
{
	free(((array_hdr_t*) self) - 1);
}

void arrayClear(void* self)
{
	array_hdr_t* hdr = ((array_hdr_t*) self) - 1;
	hdr->len = 0;
}

void* (arrayPush)(void* self, size_t size)
{
	array_hdr_t* hdr = ((array_hdr_t*) self) - 1;
	
	if (hdr->len == hdr->cap)
	{
		hdr->cap *= 2;
		hdr = realloc(hdr, sizeof(*hdr) + size*hdr->cap);
		
		if (hdr == NULL)
		{
			fputs("out-of-memory error\n", stderr);
			exit(-1);
		}
	}
	
	++hdr->len;
	return hdr + 1;
}

void (arrayPop)(void* self)
{
	array_hdr_t* hdr = ((array_hdr_t*) self) - 1;
	assert(hdr->len > 0);
	--hdr->len;
}

int arrayIsEmpty(const void* self)
{
	return ((array_hdr_t*) self)[-1].len == 0;
}

unsigned int arrayCount(const void* self)
{
	return ((array_hdr_t*) self)[-1].len;
}
