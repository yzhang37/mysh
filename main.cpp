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

#define QUOTE 		1
#define DQUOTE		2


#define EXIT 			1

#define PIPE_IN			0
#define PIPE_OUT		1
#define REDIRECT_IN		2
#define REDIRECT_OUT	3
#define INFILEPARAM O_CREAT | O_RDONLY, 0666
#define OUTFILEPARAM O_CREAT | O_WRONLY | O_TRUNC, 0666

struct command_info
{
	int type, argc;
	char *input, *output, *cmd, **param;
};

struct command_group
{
	int first, last;
};

void deal_lf(char *);
size_t parse_group (char *);
void parse_command (char *, int mode);
void make_prompt (char *);
void read_command(char *, char *);
int run_shell();
void run_command(int, int, const int &);
void reset_cmd();
int next_cmd();
int main(int argc, char **argv);
int handle(FILE *stream);
int checkinternal(const char *buf);
void endstream();
void closeFile(int &fileno);

int curCmdIndex = 0;
struct command_info Commands[MAX_CMD];
char prompt_info[MAX_STRING];
int fd[4] = {0} ;
//fd[0]: PIPE IN
//fd[1]: PIPE OUT
//fd[2]: redirection in
//fd[3]: redircetion out
int echo = 1;
int ifwait = 1;
std::set <pid_t> waitpids;
char PIPE_FILE[MAX_STRING];


void closeFile(int &fileno)
{
	if (fileno != 0)
	{
		close(fileno);
		fileno = 0;
	}
}

int main(int argc, char **argv)
{
	//signal(SIGCHLD,SIG_IGN);
	sprintf(PIPE_FILE, ".myshpip%x.tmp", rand());
	srand(time(0));
	memset(Commands, 0, sizeof(Commands));
	fd[PIPE_IN] = open(PIPE_FILE, O_CREAT | O_RDONLY, 0666);
	fd[PIPE_OUT] = open(PIPE_FILE, O_CREAT | O_WRONLY | O_TRUNC, 0666);
	if (argc == 1)
	{
		// Interactive mode
		echo = 1;
		handle(stdin);
	}
	else
	{
		// Batch mode
		echo = 0;
		for (int arg_i = 1; arg_i < argc; ++arg_i)
		{
			FILE *mystdin = fopen(argv[arg_i], "r");
			handle(mystdin);
			fclose(mystdin);
		}
	}
	endstream();
	return 0;
}

void endstream()
{
	closeFile(fd[PIPE_IN]);
	closeFile(fd[PIPE_OUT]);
	remove(PIPE_FILE);
}

int handle(FILE *stream)
{
	//by default, echo is on
	char command_buf[BUFF];
	make_prompt(prompt_info);
	while (TRUE)
	{
		if (echo)
			printf("%s", prompt_info);
		if (fgets(command_buf, BUFF, stream) == NULL)
			return 1;
		deal_lf(command_buf);

		size_t startI = 0;
		while (TRUE)
		{
			size_t pos = parse_group(command_buf + startI);
			if (pos == 0)
				break;
			startI += pos;
			switch (run_shell())
			{
			case EXIT:
				return EXIT;
			default:
				break;
			}
		}
		*command_buf = '\0';
	}
}

size_t parse_group(char *buf)
{
	size_t cnt = strlen(buf);
	if (cnt == 0) return 0;
	size_t i = 0, startPos = 0;
	ifwait = 1;
	reset_cmd();
	for (; i < cnt; ++i)
	{
		if (buf[i] == '|' || buf[i] == '&' || i == cnt - 1)
		{
			int pipe = 0;
			if (buf[i] == '|')
			{
				//Set EOL (End of Line)
				buf[i] = '\0';
				pipe = 1;
			}
			else if (buf[i] == '&')
			{
				buf[i] = '\0';
				ifwait = 0;
			}
			parse_command(buf + startPos, pipe);
			if (buf[i] == '&')
			{
				return i + 1;
			}
			startPos = i + 1;
		}
	}
	return i;
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
	memset(Commands+index, 0, sizeof(command_info));
	if (mode == PIPE)
		Commands[index].type |= PIPE;
	char *argsList[MAX_ARGS];
	int argCount = 0;

	//now this program should deal with quotes
	int quoteMode = 0;
	int quoteBegin = 0;
	for (; i < cnt; ++i)
	{
		if (command[i] == ' ' || i == cnt - 1)
		{
			if (command[i] == ' ')
			{
				if (quoteMode != 0)
					continue;
				command[i] = '\0';
			}
			if (command[j] != '\0')
			{
				if (!quoteMode)
				{
					argsList[argCount++] = command + j;
				}
				else
				{
					if (command[cnt - 1] == '\"' || command[cnt - 1] == '\'')
						command[cnt - 1] = '\0';
					argsList[argCount++] = command + quoteBegin + 1;
				}
			}

			j = i + 1;

			//remove more spaces
			while (command[j] == ' ' || command[j] == '\t') {++i; ++j;}
		}
		else if (command[i] == '\'')
		{
			switch (quoteMode)
			{
			case 0:
				quoteMode = QUOTE;
				quoteBegin = i;
				break;
			case QUOTE:
				if (i > 0 && command[i - 1] != '\\')
				{
					command[i] = '\0';
					argsList[argCount++] = command + quoteBegin + 1;
					j = i + 1;
					quoteMode = 0;
					while (command[j] == ' ' || command[j] == '\t') {++i; ++j;}
				}
				break;
			case DQUOTE:
				break;
			}
		}
		else if (command[i] == '\"')
		{
			switch (quoteMode)
			{
			case 0:
				quoteMode = DQUOTE;
				quoteBegin = i;
				break;
			case QUOTE:
				break;
			case DQUOTE:
				if (i > 0 && command[i - 1] != '\\')
				{
					command[i] = '\0';
					argsList[argCount++] = command + quoteBegin + 1;
					j = i + 1;
					quoteMode = 0;
					while (command[j] == ' ' || command[j] == '\t') {++i; ++j;}
				}
				break;
			}
		}
	}
	//now we generate cmd parameters.
	//because argsList may exists redirections, we must handle them
	int temp = 0;
	if (index != 0 && checkinternal(argsList[0]))
		return;
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
				c == '<' ? (Commands[index].input = argsList[++i]) : (Commands[index].output = argsList[++i]);
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
	Commands[index].argc = temp;
}

void reset_cmd()
{
	if (curCmdIndex != 0)
	{
		for (int i = 0; i < curCmdIndex; ++i)
		{
			if (Commands[i].param != 0)
			{
				free(Commands[i].param);
				Commands[i].param = 0;
			}
		}
	}
	curCmdIndex = 0;
}

int next_cmd()
{
	return curCmdIndex++;
}

int checkinternal(const char *buf)
{
	if (strcmp(buf, "cd") == 0)
		return 1;
	else if (strcmp(buf, "wait") == 0)
		return 1;
	else if (strcmp(buf, "exit") == 0)
		return 1;
	else if (strcmp(buf, "quit") == 0)
		return 1;
	else
		return 0;
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

	if (strlen(cwd) < strlen(pwd->pw_dir) ||
	        strncmp(cwd, pwd->pw_dir, strlen(pwd->pw_dir)))
	{
		sprintf(prompt, "%s@%s:%s", pwd->pw_name, userinfo, cwd);
	}
	else
	{
		sprintf(prompt, "%s@%s:~%s", pwd->pw_name, userinfo, cwd + strlen(pwd->pw_dir));
	}

	if (getuid() == 0)
		sprintf(prompt + strlen(prompt), "# ");
	else
		sprintf(prompt + strlen(prompt), "$ ");
}

void deal_lf(char *buf)
{
	size_t i = 0;
	unsigned char c;
	while ((c = buf[i++]) != '\0')
	{
		if (c == '\n')
		{
			buf[i - 1] = '\0';
			return;
		}
	}
}

int run_shell()
{
	for (int i = 0; i < 1; ++i)
	{
		if (strcmp(Commands[i].cmd, "cd") == 0)
		{
			if (Commands[i].argc == 1)
				chdir(getenv("HOME"));
			else
			{
				if (chdir(Commands[i].param[1]) == -1)
				{
					printf("%s: Path doesn't exists.\n", Commands[i].param[1]);
				}
			}
			make_prompt(prompt_info);
			if (curCmdIndex == 1)
				return 0;
		}
		else if (strcmp(Commands[i].cmd, "exit") == 0 ||
		         strcmp(Commands[i].cmd, "quit") == 0)
		{
			return 1;
		}
		else if (strcmp(Commands[i].cmd, "wait") == 0)
		{
			std::set <pid_t>::iterator it, oit;
			for (it = waitpids.begin(); it != waitpids.end();)
			{
				pid_t value = *it;
				waitpid(value, NULL, 0);
				it++;
				waitpids.erase(value);
				closeFile(fd[REDIRECT_IN]);
				closeFile(fd[REDIRECT_OUT]);
			}
			return 0;
		}
	}
	//OPEN TWO FILE_NUMBER
	pid_t pid = fork();
	switch (pid)
	{
	case -1:
		perror("Failed to create process.");
		exit(1);
	case 0:
		run_command(0, curCmdIndex - 1, curCmdIndex - 1);
	default:
		if (ifwait)
		{
			wait(NULL);
			closeFile(fd[REDIRECT_IN]);
			closeFile(fd[REDIRECT_OUT]);
		}
		else
		{
			if (!waitpids.count(pid))
				waitpids.insert(pid);
		}
	}
	return 0;
}

void run_command(int start, int end, const int &lastEnd)
{
	pid_t pid = 0;
	//first we assume it's not internal command
	int isinternal = 0;
	if (start != end)
	{
		// first here we detect
		// whether there is a internal command
		if (checkinternal(Commands[end].cmd))
		{
			isinternal = 1;
			// because it is internal cmd, we don't need to fork a new child process.
			run_command(start, end - 1, lastEnd);
		}
		else
		{
			pid = fork();
			switch (pid)
			{
			case -1:
				perror("Failed to create process.");
				exit(1);
			case 0:
				run_command(start, end - 1, lastEnd);
			default:
				wait(NULL);
				if (end == start + 1)
				{
					//close the possible redirect in file.
					closeFile(fd[REDIRECT_IN]);
				}
				//close the last possible redirection out file.
				closeFile(fd[REDIRECT_OUT]);
			}
		}
	}

	//if this program is not the first program?
	if (end != start)
		dup2(fd[PIPE_IN], STDIN_FILENO);
	else
	{
		if (Commands[start].input != NULL)
		{
			fd[REDIRECT_IN] = open(Commands[start].input, INFILEPARAM);
			dup2(fd[REDIRECT_IN], STDIN_FILENO);
		}
	}

	if (Commands[end].output != NULL)
	{
		fd[REDIRECT_OUT] = open(Commands[end].output, OUTFILEPARAM);
		dup2(fd[REDIRECT_OUT], STDOUT_FILENO);
	}

	if (end != lastEnd)
		dup2(fd[PIPE_OUT], STDOUT_FILENO);
	if (isinternal == 1)
	{
		if (strcmp(Commands[end].cmd, "cd") == 0 && end != start)
		{
			printf("Invaild cd parameter.\n");
		}
		else if (strcmp(Commands[end].cmd, "wait") == 0)
		{
			;
		}
		else if (strcmp(Commands[end].cmd, "exit") == 0 ||
		         strcmp(Commands[end].cmd, "quit") == 0)
		{
			;
		}
	}
	else
		execvp(Commands[end].cmd, Commands[end].param);
		perror(Commands[end].cmd);
}
