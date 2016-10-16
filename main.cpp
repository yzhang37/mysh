/*
This file is written by yzhang37
Email: 10142130151_ecnu@outlook.com

*/

#include "stdafx.h"
#include "command.h"

#define TRUE (-1)
#define FALSE (0)
#define MAX_ARGS	50
#define MAX_CMD  	200
#define MAX_GRP		100
#define BUFF		4096
#define MAX_STRING	200

#define PIPE					1
#define BACKGROUND				2
#define IN_REDIRECT				4
#define OUT_REDIRECT 			8
#define OUT_REDIRECT_APPEND		16

struct command_info
{
	int type;
	char *input, *output, *cmd, **param;
};

struct command_group
{
	int first, last;
};

void deal_lf(char *);
void parse_group (struct command_group *, char *);
void parse_command (char *, int mode);
void make_prompt (char *);
void read_command(char *, char *);
void run_shell();
void run_command(int, int);
void reset_cmd();
int next_cmd();
int main(int argc, char **argv);
int handle(FILE *stream);

int curCmdIndex = 0;
struct command_info Commands[MAX_CMD];

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
	char command_buf[BUFF];
	make_prompt(prompt_info);
	while (TRUE)
	{
		if (echo)
			printf("%s", prompt_info);
		fgets(command_buf, BUFF, stream);
		command_group group[MAX_GRP];
		deal_lf(command_buf);

		printf("%s", command_buf);
	}
}

void parse_group(struct command_group *grp, char *buf)
{
	size_t cnt = strlen(buf);
	size_t i = 0, startPos = 0;
	for (size_t i = 0; i < cnt; ++i)
	{
		if (buf[i] == '|' || i == cnt - 1)
		{
			int mode = 0;
			if (buf[i] == '|')
			{
				//Set EOL (End of Line)
				buf[i] = '\0';
				mode = 1;
			}

			startPos = i+1;
		}
	}
}

void parse_command (char *command, int mode)
{
	size_t cnt = strlen(command);
	size_t i = 0, j = 0;
	while (command[i] == ' ' || command[i] == '\t')
		++i; ++j;
	size_t index = next_cmd();
	Commands[index].type = 0;
	if (mode == PIPE)
		Commands[index].type |= PIPE;
	char *argsList[MAX_ARGS];

	for (int startX = 0; i < cnt; ++i)
	{
		if (command[i] == ' ' || i == cnt - 1)
		{
			if (command[i] == ' ')
				command[i] = '\0';
			argsList[startX++] = command + j;
			
			j = i+1;

			//remove more spaces
			while (command[j] == ' ' || command[j] == '\t')
				++i; ++j;
		}
	}
}

void reset_cmd()
{
	curCmdIndex = 0;
}

int next_cmd()
{
	return curCmdIndex++;
}

void make_prompt(char *prompt)
{
	char userinfo[MAX_STRING], cwd[MAX_STRING];
	struct passwd* pwd = getpwuid(getuid());
	//get current work path
	getcwd(cwd, MAX_STRING);

	if (gethostname(userinfo, MAX_STRING))
	{
		strcpy(userinfo, "unknown");
	}

	if (strlen(cwd)<strlen(pwd->pw_dir) ||
		strncmp(cwd, pwd->pw_dir, strlen(pwd->pw_dir)))
	{
		sprintf(prompt, "%s@%s:%s", pwd->pw_name, userinfo, cwd);
	}
	else
	{
		sprintf(prompt, "%s@%s:~%s", pwd->pw_name, userinfo, cwd + strlen(pwd->pw_dir));
	}

	if (getuid() == 0) 
		sprintf(prompt+strlen(prompt), "# ");
	else
		sprintf(prompt+strlen(prompt), "$ ");
}

void deal_lf(char *buf)
{
	size_t i = 0;
	unsigned char c;
	while ((c=buf[i++]) != '\0')
	{
		if (c == '\n')
		{
			buf[i-1] = '\0';
			return;
		}
	}
}