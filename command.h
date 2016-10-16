#include "stdafx.h"

#define MAX_ARGS 50
#define MAX_CMD  1000
#define MAX_GRP 100
#define BUFF 4096
#define MAX_STRING 200

void parse_group (char *);
void parse_command (char *, int mode);
void make_prompt (char *);
void read_command(char *, char *);
void run_shell();
void run_command(int, int);
void clear_cmd(void);
int init_cmd(void);

struct command_info
{
	int type;
	char *input, *output, *cmd, **param;
};

struct command_group
{
	int first, last;
};

struct command_info cmd[MAX_CMD];
struct command_group group[MAX_GRP];