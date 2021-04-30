#include "shell.h"

// Create a command
command* create_command(int type)
{
    command *cmd = malloc(sizeof(command));
    cmd->argv = NULL;
    cmd->type = type;
    cmd->argc = 0;
    cmd->background = 0;
    cmd->process_number = -1;

    return cmd;
}

// Create a list of command list node
command_list* create_command_list_node(int type)
{
    command_list* list = malloc(sizeof(command_list));
    list->cmd = create_command(type);
    list->next = NULL;

    return list;
}

void delete_command_list_node(command_list* current, int cmd)
{
    int i = 0;    

    if(current == NULL)
        return;

    // Free the cmd
    if(current->cmd != NULL)
    {
        if(cmd)
        {
            if(current->cmd->argv != NULL)
            {
                for(i = 0; current->cmd->argv[i] != NULL; i++)
                {
                    if(current->cmd->argv[i] != NULL)
                        free(current->cmd->argv[i]);
                }
                free(current->cmd->argv);
            }
            free(current->cmd);
        }
    }
    if(current != NULL)
        free(current);

    return;
}


// Delete a list of commands
void delete_command_list(command_list* list)
{
    command_list *current = NULL, *next = NULL;
    current = list;
    int i = 0;
    while (current != NULL)
    {
        // Save the next command
        next = current->next;
        delete_command_list_node(current, 1);

        // Move current to next
        current = next;
    } 
}

// Look for various exit commands
int is_exit_command(char *input)
{
    if(strcmp(input, "Exit()") == 0 || strcmp(input, "Quit()") == 0)
    {
        return 1;
    }
    else if(strcmp(input, "exit()") == 0 || strcmp(input, "quit()") == 0)
    {
        return 1;
    }
    else if(strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0)
    {
        return 1;
    }
    else if(strcmp(input, "Exit") == 0 || strcmp(input, "Quit") == 0)
    {
        return 1;
    }
    else if(strcmp(input, "close") == 0 || strcmp(input, "Close") == 0)
    {
        return 1;
    }
    
    return 0;
}

// Process input seperates input into a list of args called argv
// Returns argv and sets argc
command_list* process_input(char *input, int *argc)
{ 
    char **argv, *arg; 
    int position = 0, commands = 0;
    int total = 64, size = 64;    
    command *cmd;

    // Create the command list
    command_list *head;
    command_list *current = NULL;
    head = create_command_list_node(HEAD);
    
    // Exit to close the shell
    if(is_exit_command(input))
    {
        head->cmd->type = EXIT;
        return head;
    }

    head->next = create_command_list_node(NORMAL);
    current = head->next;

    commands++;

    // Split input into vector of arguments (base vector is 64 arguments long)
    argv = malloc(64 * sizeof(char*));
    if(argv == NULL)
    {
        fprintf(stderr,"Input Processing Error: failed to allocate memory for argv\n");
        exit(EXIT_FAILURE);
    }
    
    // Split the input on space, tab, carriage return, new line (IF THIS BREAKS NEED \a IN THE STRING)
    arg = strtok(input, " \t\r\n");

    // While the current split is not NULL
    while (arg != NULL) 
    {
        if(current->cmd->type == TAIL)
        {
            current->cmd->type = NORMAL;
        }

        // If the command is history
        if(strcmp(arg, "history") == 0)
            current->cmd->type = HISTORY;

        // If the command is help
        if(strcmp(arg, "help") == 0)
            current->cmd->type = HELP;

        // If the command is jobs
        if(strcmp(arg, "jobs") == 0)
            current->cmd->type = JOBS;

        // If the command is ls
        if(strcmp(arg, "ls") == 0)
            current->cmd->type = LS;

        // If the command is cd
        if(strcmp(arg, "cd") == 0)
            current->cmd->type = CD;

        // If run in background operator is found
        if(strcmp(arg, "&") == 0)
        {
            // Set the last position of the argv as NULL and add to command
            argv[position] = NULL;
            current->cmd->argv = argv;

            // Set the number of arguments and to run in background
            current->cmd->argc = position;
            current->cmd->background = 1;

            // Add command to commands
            current->next = create_command_list_node(TAIL);
            current = current->next;

            // Create new command and arguement vector
            argv = malloc(64 * sizeof(char*));   

            // Reset the number of arguements for the next command and increase number of commands
            position = 0;
            commands++;     
        }
        else
        {
            // Set the current position in the vector to arg and increase position
            argv[position] = malloc(sizeof(char)*(strlen(arg)+1));
            current->cmd->argv = argv;
            memcpy(argv[position++], arg, strlen(arg) + 1);
        }

        // If position has reached the current max vector length
        if (position >= total) 
        {
            // Increase vector length
            total += size;

            // Reallocate memory for the vector of arguments
            argv = realloc(argv, total * sizeof(char*));

            // If the realloc fails then print error and exit
            if (argv == NULL) 
            {
                fprintf(stderr, "Input Processing Error: failed to allocate memory for argv\n");
                exit(EXIT_FAILURE);
            }
        }

        // Get the next input split on space, tab, carriage return, new line (IF THIS BREAKS NEED \a IN THE STRING)
        arg = strtok(NULL, " \t\r\n");
    }

    // Set the last position to NULL and add to the command
    argv[position] = NULL;
    current->cmd->argv = argv;

    // Set number of command arguements
    current->cmd->argc = position;

    // Set the number of commands to position
    *argc = commands;

    // Return the argv
    return head;
}