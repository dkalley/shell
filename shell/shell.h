#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/mman.h>

// Program Constants
#define HEAD 1
#define NORMAL 2
#define CD 3
#define LS 4
#define HELP 5
#define JOBS 6
#define HISTORY 7
#define TAIL 8
#define EXIT 9

// Struct Definitions
typedef struct command {
    char    **argv;             // List of arguments
    int     type;               // Type of command
    int     argc;               // Number of arguments
    int     background;         // If & - Perform in background
    int     process_number;     // Process number = number of process currenlty running
} command;

typedef struct command_list
{
    command             *cmd;   // Command
    struct command_list *next;  // Next command
} command_list;

typedef struct running_commands
{
    int running[10];
    command *cmds[10];
} running_commands;

// Program Shared Variables 
static int ctrl_c_flag = 1;
static char prompt[5000] = {0};
running_commands *bg;
static int prompt_flag = 0;

// Check if base of cwd is equal to /home/[uid]
int check_dir(char *cwd, char *uid)
{
    int uid_len = strlen(uid), return_val = 0;
    char temp[100] = {0};
    char test[100] = "/home/";

    // If cwd is at least equal to minimum length check to see if base exists
    if(strlen(cwd) >= 6+uid_len)
    {
        strncpy(temp, cwd, 6+uid_len);
        strcpy(test+6, uid);

        return_val = (strcmp(temp, test) == 0);
    }

    return return_val;
}

// Replace /home/[UID] -> ~
void fix_cwd_string(char *cwd, char *cwd_w_t, char *uid, char *cid)
{
    int offset = 0;

    // Get Offset
    offset = strlen(uid) + strlen(cid) + 2;

    // Reset cwd_w_t
    bzero(cwd_w_t, 4096);

    // If /home/[uid] then replace otherwise
    if(check_dir(cwd, uid))
    {
        cwd_w_t[0] = '~';
        strcpy(cwd_w_t+1, cwd+offset);
    }
    else
       strcpy(cwd_w_t, cwd); 

    sprintf(prompt, "%s@%s:%s$ ", uid, cid, cwd_w_t);
}

