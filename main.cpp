/*
This file is written by yzhang37
Email: 10142130151_ecnu@outlook.com

*/

#include "stdafx.h"
#include "command.h"

#define TRUE (-1)
#define FALSE (0)

char prompt[MAX_STRING] = "mysh >";

int main(int argc, char **argv);
int handle(FILE *stream);

int main(int argc, char **argv)
{
	if (argc == 1)
	{
		// Interactive mode
		handle(stdin);
	}
	else 
	{
		// Batch mode
		for (int arg_i = 1; arg_i < argc; ++arg_i)
		{

		}
	}
	return 0;
}

int handle(FILE *stream)
{
	//by default, echo is on
	int echo = 1;
	char prompt_info[MAX_STRING];
	make_prompt(prompt_info);
	while (TRUE)
	{
		if (echo)
			puts(prompt_info);
	}
}
