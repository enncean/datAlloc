#include <stdio.h>
#include <string.h>

#include "datAlloc/datAlloc.h"

static void dumpHeapData (unsigned int rangeStart, unsigned int rangeEnd);

int main ()
{
	char *a = NULL;
	short *b = NULL;
	int *c = NULL;
	
	printf("==== datAlloc Sample ====\n\n");
	
	a = dataMalloc(16);
	if (a != NULL)
	{
		memcpy(a, "datAlloc Sample", 16);
		printf("a(%p)=%s\n", a, a);
	}
	
	b = dataMalloc(2);
	if (b != NULL)
	{
		*b = 15;
		printf("b(%p)=%d\n", b, *b);
	}
	dumpHeapData(0, 64);
	
	printf("\nfree:a\n");
	if (a != NULL)	dataFree(a);
	dumpHeapData(0, 64);
	
	printf("\n");
	c = dataMalloc(4);
	if (c != NULL)
	{
		*c = 0xABCD;
		printf("c(%p)=%X\n", c, *c);
	}
	dumpHeapData(0, 64);
	
	if (b != NULL)	dataFree(b);
	if (c != NULL)	dataFree(c);
	
	dumpHeapData(0, 64);
	
	return 0;
}

static void dumpHeapData (unsigned int rangeStart, unsigned int rangeEnd)
{
#ifdef DA_DEBUG
	HeapDataReport report;
	unsigned int i;
	
	getHeapReport(&report);
	printf("\nSize:%d, Used:(N:%d, Size:%d)\n",
		   report.memorySize, report.usedCount, report.usedSize);
	
	if (rangeStart < rangeEnd)
	{
		if (rangeStart % 16 != 0)
		{
			rangeStart /= 16;
		}
		if (rangeEnd > report.memorySize)
		{
			rangeEnd = report.memorySize;
		}
		
		for (i = rangeStart; i < rangeEnd; i++)
		{
			if ((i != 0) && (i % 16 == 0))
			{
				printf("\n");
			}
			printf("%02X ", report.heapBuffer[i]);
		}
		printf("\n");
	}
#endif
}