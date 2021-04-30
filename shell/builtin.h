#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

// help - Prints a list of builtin commands 
void my_help()
{
    printf("Daniel Kalley's Shell version 1.0\n");
    printf("These shell commands are defined internally.  Type `help' to see this list.\n");
    printf("Use `man' or `info' to find out more about commands not in this list.\n\n");
    printf("cd [dir]\t- Change current directory to dir\n");
    printf("jobs    \t- List current running jobs\n");
    printf("history \t- List history of commands\n");
    printf("help    \t- List built-in commands\n");
    printf("exit    \t- Exit the shell\n");
    fflush(stdout);
}

// cd - Changes current directory to given directory
void change_director(char **argv, char *cwd, char *uid, char *cid, char *cwd_w_t)
{
    char default_dir[100];
    
    // If Command is just cd 
    if(argv[1] == NULL)
    {
        sprintf(default_dir,"/home/%s/",getenv("USER"));  
        if(chdir(default_dir) != 0)
            perror("CD error");
    }
    else
    {
        if(chdir(argv[1]) != 0)
            perror("CD error");
    }

    bzero(cwd, 4096);
    getcwd(cwd, 4096);
    fix_cwd_string(cwd, cwd_w_t, uid, cid);
    sprintf(prompt, "%s@%s:%s$ ", uid, cid, cwd_w_t);
}

// history - Print a list of past commands
void my_history()
{
    HIST_ENTRY **the_list;
    int i;

    the_list = history_list();

    if(the_list)
        for (i = 0; the_list[i]; i++)
          printf ("%d: %s\n", i + history_base, the_list[i]->line);
}

// jobs - Print current working jobs with thier process number
void my_jobs()
{
    int i = 0, j = 0, k = 0, idx = 0;
    command* cmd;
    char co[1024];

    // Print all active jobs
    for(idx = 0; idx < 10; idx++)
    {
        if(bg->running[idx])
        {
            cmd = bg->cmds[idx];            

            // Create command string
            for(i = 0, k = 0; cmd->argv[i] != NULL; i++)
            {
                for(j = 0; cmd->argv[i][j] != '\0'; j++)
                    co[k++] = cmd->argv[i][j];
                co[k++] = ' ';        
            }
            co[k] = '\0';

            printf("[%d] Running \t\t%s\n",idx+1,co);
            fflush(stdout);
        }        
    }    
}

// ls - print current directory contents
void my_ls(char **argv)
{
    pid_t pid, wpid;
    int status;
    char path[1024];
    FILE *fp;
    
    // Spawn a child process
    pid = fork();

    // If the current process is the child
    if(pid == 0)
    {
        status = execvp(argv[0], argv);

        if(status == -1)
        {
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }

    // If the fork failed
    else if(pid < 0)
    {
        fprintf(stderr,"Launch Error\n");
        perror("Launch Error");
    }

    // If the current process is the parent
    else
    {
        // While the child process is active
        do 
        {
            // Get the status of teh child process
            wpid = waitpid(pid, &status, WUNTRACED);
        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    // Turn ctrl c prints back on
    ctrl_c_flag = 1;

    return;

}
