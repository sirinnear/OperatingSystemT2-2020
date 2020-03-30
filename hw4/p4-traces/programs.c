/*
Do not modify this file.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static int compare_bytes(const void *pa, const void *pb)
{
	int a = *(char *)pa;
	int b = *(char *)pb;

	if (a < b)
	{
		return -1;
	}
	else if (a == b)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

void focus_program(char *data, int length)
{
	int total = 0;
	int i, j;

	srand(38290);

	for (i = 0; i < length; i++)
	{
		data[i] = 0;
	}

	for (j = 0; j < 100; j++)
	{
		int start = rand() % length;
		int size = 25;
		for (i = 0; i < 100; i++)
		{
			data[(start + rand() % size) % length] = rand();
		}
	}

	for (i = 0; i < length; i++)
	{
		total += data[i];
	}

	printf("focus result is %d\n", total);
}

void sort_program(char *data, int length)
{
	int total = 0;
	int i;

	srand(4856);

	for (i = 0; i < length; i++)
	{
		data[i] = rand();
	}

	qsort(data, length, 1, compare_bytes);

	for (i = 0; i < length; i++)
	{
		total += data[i];
	}

	printf("sort result is %d\n", total);
}

void scan_program(char *cdata, int length)
{
	unsigned i, j;
	unsigned char *data = cdata;
	unsigned total = 0;

	for (i = 0; i < length; i++)
	{
		data[i] = i % 256;
	}

	for (j = 0; j < 10; j++)
	{
		for (i = 0; i < length; i++)
		{
			total += data[i];
		}
	}

	printf("scan result is %d\n", total);
}

int main(int argc, char *argv[])
{
	int c;
	char *prog = NULL;
	int buf_size = 1000;
	while ((c = getopt(argc, argv, "p:s:")) != -1)
	{
		switch (c)
		{
		case 'p':
			prog = optarg;
			break;
		case 's':
			buf_size = atoi(optarg);
			break;
		default:
			return 1;
		}
	}
	if (!prog)
	{
		printf("Please specify a program name.\n");
	}
	else
	{
		char *buf = (char *)malloc(buf_size * sizeof(char));
		if (strcmp(prog, "scan") == 0)
		{
			printf("found scan");
			scan_program(buf, buf_size);
		}
		else if (strcmp(prog, "sort") == 0)
		{
			printf("found sort");
			sort_program(buf, buf_size);
		}
		else if (strcmp(prog, "focus") == 0)
		{
			printf("found focus");
			focus_program(buf, buf_size);
		}
		free(buf);
	}
}
