#include "shell/input.h"
#include "shell/builtin.h"
#include "shell/execute.h"

void clean_up_memory();

// Create look similar to commandline
// Ex. [User ID]@[Computer Name]:[Current Working Directory]
void shell()
{
    char *uid, cid[100] = {0}, cwd[4096] = {0}, cwd_w_t[4096] = {0};
    char *input, **argv;
    int exit_code = 0, i = 0, process_id = 0, commands = 0;
    command **cmds;
    command_list *head, *current, *next;

    // Implement the tab autofill feature
    // Bind tab to auto-complete
    rl_bind_key('\t', rl_complete);

    // Get user id, computer name, and cwd
    uid = getenv("USER");
    gethostname(cid, 100);
    getcwd(cwd, 4096);

    // Replace /home/[UID] -> ~ if it exists
    fix_cwd_string(cwd, cwd_w_t, uid, cid);

    // mmap to share the background struct with spawned processes
    bg = mmap(NULL, sizeof(*bg),PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    // Initialize background struct
    for(i = 0; i < 10; i++)
    { 
        bg->running[i] = 0;
        bg->cmds[i] = NULL;
    }

    // Wait for commands, loop until exit_code
    while(!exit_code)
    {
        // Get command
        prompt_flag = 1;
        input = readline(prompt);
        if(!input)
            break;
        
        if(input[0] == '\0')
            continue;

        // Retain a history of previous commands to use with arrow
        add_history(input);

        // Process the input into a vector of arguments
        head = process_input(input, &commands); 
        
        // Loop over all commands given
        for(current = head; current != NULL && exit_code == 0; current = next)
        {          
            // Save the next command
            next = current->next;   

            // Determine what the command needs to do   
            switch(current->cmd->type)
            {        
                case HEAD:      break;
                case NORMAL:    execute(current, commands); break;
                case CD:        change_director(current->cmd->argv, cwd, uid, cid, cwd_w_t); break;
                case LS:        my_ls(current->cmd->argv); break;
                case HELP:      my_help(); break;
                case JOBS:      my_jobs(); break;
                case HISTORY:   my_history(); break;
                case TAIL:      break;                
                case EXIT:      exit_code = 1; break;
                default:        break;
            }    

            // If current command is not running in the background free it completely
            if(!current->cmd->background)
            {
                bg->cmds[current->cmd->process_number] = NULL;
                delete_command_list_node(current, 1);
            }
            else
                delete_command_list_node(current, 0);
        }

        free(input);
    }

    // Unmap the shared memory
    munmap(bg,sizeof(*bg));
}

// Ctrl-c to terminate a running program and not the shell
void ctrl_c_handler(int sig)
{
    // Signal that ctrl c was pressed
    if(ctrl_c_flag == 1)
        printf("\n%s", prompt);
    else
    {
        printf("\n");
    }
}

int main()
{
    // Set the ctrl-c signal to redirect to my hanlder
    signal(SIGINT, ctrl_c_handler);

    // Launch the shell
    shell();

    return 0;
}

