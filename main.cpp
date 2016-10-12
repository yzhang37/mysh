/*
This file is written by yzhang37
Email: 10142130151_ecnu@outlook.com

*/

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <string.h>
#include <unistd.h>

#define TRUE (-1)
#define FALSE (0)

#define MAX_ARGS (50) //the max count of args
#define MAX_CMD (1000)
#define MAX_GRP (100)
#define BUFF (4096)




char prompt[MAX_STRING] = "mysh >";

int main(int argc, char **argv);
int handle(FILE *stream);

int main(int argc, char **argv)
{
	while (true)
	{
		
	}
	return 0;
}

int handle(FILE *stream)
{
	while (TRUE)
	{

	}
}
