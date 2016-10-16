/*
This file is written by yzhang37
Email: 10142130151_ecnu@outlook.com

*/

#include "stdafx.h"

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
void parse_group (char *);
void parse_command (char *, int mode);
void make_prompt (char *);
void read_command(char *, char *);
void run_shell();
void run_command(int, int, const int &);
void reset_cmd();
int next_cmd();
int main(int argc, char **argv);
int handle(FILE *stream);

int curCmdIndex = 0;
struct command_info Commands[MAX_CMD];
int fd[2];

int main(int argc, char **argv)
{
	srand(time(0));
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
		deal_lf(command_buf);
		parse_group(command_buf);
		run_shell();
	}
}

void parse_group(char *buf)
{
	size_t cnt = strlen(buf);
	size_t i = 0, startPos = 0;
	reset_cmd();
	for (size_t i = 0; i < cnt; ++i)
	{
		if (buf[i] == '|' || i == cnt - 1)
		{
			int pipe = 0;
			if (buf[i] == '|')
			{
				//Set EOL (End of Line)
				buf[i] = '\0';
				pipe = 1;
			}
			parse_command(buf+startPos, pipe);
			startPos = i+1;
		}
	}
}

void parse_command(char *command, int mode)
{
	size_t cnt = strlen(command);
	size_t i = 0, j = 0;
	while (command[i] == ' ' || command[i] == '\t')
	{
		++i; ++j;
	}
	size_t index = next_cmd();
	Commands[index].type = 0;
	if (mode == PIPE)
		Commands[index].type |= PIPE;
	char *argsList[MAX_ARGS];
	int argCount = 0;
	for (; i < cnt; ++i)
	{
		if (command[i] == ' ' || i == cnt - 1)
		{
			if (command[i] == ' ')
				command[i] = '\0';
			argsList[argCount++] = command + j;
			
			j = i+1;

			//remove more spaces
			while (command[j] == ' ' || command[j] == '\t')
			{
				++i; ++j;
			}
		}
	}
	//now we generate cmd parameters.
	//because argsList may exists redirections, we must handle them
	int temp = 0;
	Commands[index].cmd = argsList[0];
	Commands[index].param = (char **)malloc(sizeof(char *) * (argCount + 2));

	if (argCount > 0)
		Commands[index].param[temp++] = argsList[0];
	for (i = 1; i < argCount; ++i)
	{
		int isredir = 0;
		if (strlen(argsList[i]) == 1)
		{
			//probable: < and >
			char c = *(argsList[i]);
			if (c == '<' || c == '>')
			{
				isredir = 1;
				//next parameters
				c == '<'? (Commands[index].input = argsList[++i]) : (Commands[index].output = argsList[++i]);
			}
		}
		else if (strlen(argsList[i]) == 2)
		{
			if (strcmp(argsList[i], "<<") == 0)
			{
				isredir = 1;
				Commands[index].input = argsList[++i];
			}
			else if (strcmp(argsList[i], ">>") == 0)
			{
				isredir = 1;
				Commands[index].output = argsList[++i];
			}
		}

		if (!isredir)
		{
			Commands[index].param[temp++] = argsList[i];
		}
	}
	Commands[index].param[temp] = (char *)0;
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

void run_shell()
{
	char PIPE_FILE[MAX_STRING];
	sprintf(PIPE_FILE, "myshpip%x.tmp", rand());

	fd[0] = open(PIPE_FILE, O_CREAT|O_RDONLY, 0666);
	fd[1] = open(PIPE_FILE, O_CREAT|O_WRONLY|O_TRUNC, 0666);

	pid_t pid = fork();

	switch (pid)
	{
		case -1:
			perror("Failed to create process.");
			exit(1);
		case 0:
			run_command(0, curCmdIndex - 1, curCmdIndex - 1);
		default:
			waitpid(0, NULL, 0);
	}
	close(fd[0]);
	close(fd[1]);
}

void run_command(int start, int end, const int &lastEnd)
{
	pid_t pid;
	if (start != end)
	{
		pid = fork();
		switch (pid)
		{
			case -1:
				perror("Failed to create process.");
				exit(1);
			case 0:
				run_command(start, end-1, lastEnd);
			default:
				waitpid(0, NULL, 0);
		}
	}

	if (end != start)
		dup2(fd[0], STDIN_FILENO);
	if (end != lastEnd -1)
		dup2(fd[1], STDOUT_FILENO);
	execvp(Commands[end].cmd, Commands[end].param);
}