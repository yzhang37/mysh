#include "command.h"
#include "stdafx.h"

const int MAX_ARGS = 50;
const int MAX_CMD = 1000;
const int MAX_GRP = 100;
const int BUFF = 4096;
const int MAX_STRING= 200;

void parse_command(char *buffer)
{
	
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