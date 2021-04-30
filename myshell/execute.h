#include <stdio.h>
#include <stdlib.h>

void execute_command(command_list *cl, int argc, int process_id);
int get_process_number();
void execute(command_list *current, int commands);
void my_history2();
// Execute a command
void execute(command_list *current, int commands)
{

    // Get the process number of the command
    int process_id = get_process_number();
    // Set the command at the 
    bg->cmds[process_id] = current->cmd;
    execute_command(current, commands, process_id);
}


// Helper functions
int get_process_number()
{
    int i = 0, j = 0;

    // Look for the first index equal to 1
    for(i = 10; i > 0; i--)
    {
        if(bg->running[i-1])
            break;
    }

    // Set the next avalible index to 1 
    bg->running[i] = 1;

    if(bg->cmds[i] != NULL)
    {
        if(bg->cmds[i]->argv != NULL)
        {
            free(bg->cmds[i]->argv);
        }
        free(bg->cmds[i]);
        bg->cmds[i] = NULL;
    }   

    // Return index
    return i;
}

// Provides support for built-in functionality "cd, ls, etc" by default
//void execute_command(command *c, int argc, int process_id)
void execute_command(command_list *cl, int argc, int process_id)
{
    pid_t pid, wpid;
    int status, i, j, k;
    FILE *fp;
    char path[1024];
    char co[1024];
    command *c = cl->cmd;

    // Turn off ctrl c prints
    ctrl_c_flag = 0;
    
    // Spawn a child process
    pid = fork();

    // If the current process is the child
    if(pid == 0)
    {
        // Create command string
        for(i = 0, k = 0; c->argv[i] != NULL; i++)
        {
            for(j = 0; c->argv[i][j] != '\0'; j++)
                co[k++] = c->argv[i][j];
            co[k++] = ' ';        
        }
        co[k] = '\0';

        c->process_number = process_id;

        if(c->background)
        {
            printf("[%d] %d\n",c->process_number+1,getpid());
            fflush(stdout);
        }    
        // Pipe the command
        fp = popen(co,"r");
        while(fgets(path, 1024, fp) != NULL)
        {
            printf("%s", path);
            prompt_flag = 0;
            fflush(stdout);
            fflush(stderr);
        }

        // Close the pipe and get return value       
        status = pclose(fp);

        // Reset the process_id flag
        bg->running[c->process_number] = 0;

        // If the process is in the background display return value
        if(c->background)
        {
            printf("[%d] Exit %d\t\t%s\n",c->process_number+1,status,co);   
            fflush(stdout);
            if(prompt_flag == 0)
            {            
                prompt_flag = 1;             
                printf("%s",prompt);
            }        
        }

        delete_command_list_node(cl, 1);

        // Figure out exit code
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
        if(!c->background)
        {
            // While the child process is active
            do 
            {
                // Get the status of teh child process
                wpid = waitpid(pid, &status, WUNTRACED);
            } while(!WIFEXITED(status) && !WIFSIGNALED(status));
        }
    }

    // Turn ctrl c prints back on
    ctrl_c_flag = 1;

    return;
}

void my_history2()
{
    HIST_ENTRY **the_list;
    int i;

    the_list = history_list();

    if(the_list)
        for (i = 0; the_list[i]; i++)
          printf ("%d: %s\n", i + history_base, the_list[i]->line);
}
