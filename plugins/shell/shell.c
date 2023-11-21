/*This module is targeted to implement a shell for the system
* It will be used to execute commands and to manage the system
* It will be implemented as a thread and will be executed in the background
* It will be possible to interact with it through the socket
* It listen on a port and wait for commands
*/

#include "shell.h"


// struct to store the command'
typedef struct command_s
{
    char *name;
    char *args[MAX_COMMAND_ARGS];
    int argc;
} command_t;

// struct to store the command list
typedef struct command_list_s
{
    command_t *commands[MAX_COMMANDS];
    int count;
} command_list_t;

// Open a socket with the given port and waiting for connections to execute commands. 
// By default the port number is 7878 if it is not configured or configured as 0.
// The port number can be configured in the config file.
// The function will return the socket if the socket is opened successfully, otherwise it will return NULL;
int open_socket(int port)
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        fprintf(stderr, "Failed to open socket\n");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;

    // if the port is not configured, use the default port 7878
    if (port == 0)
    {
        server_addr.sin_port = htons(7878);
    }
    else
    {
        server_addr.sin_port = htons(port);
    }

    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        fprintf(stderr, "Failed to bind socket\n");
        return -1;
    }

    if (listen(socket_fd, MAX_SHELL_NUMBER) < 0)
    {
        fprintf(stderr, "Failed to listen socket\n");
        return -1;
    }

    return socket_fd;
}



/* function to execute_command */
int execute_command_in_plugin(shell_t* shell, loaded_plugins_t *current_plugin,char *command_args[MAX_COMMAND_ARGS], FILE *fp){
    // check if plug have symbol execute_command via dlsym
    void * executor = dlsym(current_plugin->plugin, "execute_command");
    if (executor == NULL)
    {
        fprintf(fp, "Failed to find execute_command symbol\n");
        return -1;
    }
    // execute the command
    
    ((void (*)(shell_t *, char **, FILE *))executor)(shell, command_args, fp);
    return 0;

}

void load_plugin_from_shell(shell_t *shell, char *plugin_conf_file){
                FILE *fp = fopen(plugin_conf_file, "rb");
                if (!fp)
                {
                    fprintf(stderr, "Failed to open file %s\n", plugin_conf_file);
                    return ;
                }
                yaml_parser_wrapper_t *yaml_parser_wrapper_2 = init_yaml_parser(fp);
                if (!yaml_parser_wrapper_2)
                {
                    fprintf(stderr, "Failed to initialize yaml parser\n");
                    return ;
                }
                fclose(fp);
                yaml_node_t *root_node = get_yaml_root_node(yaml_parser_wrapper_2);
                if (!root_node)
                {
                    fprintf(stderr, "Failed to get root node\n");
                    close_yaml_parser(yaml_parser_wrapper_2);
                    return ;
                }

                yaml_node_pair_t *pair = get_yaml_node_id_by_key(yaml_parser_wrapper_2, root_node, "plugins");

                yaml_node_t *plugins_nodes = yaml_document_get_node(&yaml_parser_wrapper_2->document, pair->value);
                if (!plugins_nodes)
                {
                    fprintf(stderr, "Failed to get plugins node\n");
                    close_yaml_parser(yaml_parser_wrapper_2);
                    return ;
                }
                yaml_node_t * orig_plugins_nodes = yaml_document_get_node(&shell->yaml_parser_wrapper->document, shell->plugins_node_id);
                merge_from_file(shell->yaml_parser_wrapper, shell->plugins_node_id, yaml_parser_wrapper_2, pair->value);

                yaml_mapping_node_child_t *children = get_yaml_mapping_node_children(yaml_parser_wrapper_2, plugins_nodes);
                if (!children)
                {
                    fprintf(stderr, "Failed to get plugins node children\n");
                    close_yaml_parser(yaml_parser_wrapper_2);
                    return ;
                }
                for (int i=0; i<plugins_nodes->data.mapping.pairs.top - plugins_nodes->data.mapping.pairs.start; i++){
                    if (chain_has_key(shell->loaded_plugins, children[i].key)!=NULL){
                        fprintf(fp, "The plugin %s is already loaded\n", children[i].key);
                        continue;
                    }
                     yaml_node_t * config_node = get_yaml_node_by_key(shell->yaml_parser_wrapper,orig_plugins_nodes, children[i].key);
                    fprintf(stdout, "loading plugin %s ...\n", children[i].key);
                    void *plugin = load_plugin(shell->yaml_parser_wrapper, config_node);
                    
                    if (plugin ==NULL)  {
                        continue;
                    }

                    // initialize the plugin
                    void* ret = init_plugin(plugin, shell->yaml_parser_wrapper, config_node, shell->global_config, shell->loaded_plugins);
                    if (ret==NULL)
                    {
                        fprintf(stderr, "Failed to initialize plugin %s\n", children[i].key);
                        dlclose(plugin);
                        continue;
                    }

                    loaded_plugins_t *loaded_plugin = malloc(sizeof(loaded_plugins_t));
                    loaded_plugin->plugin = plugin;
                    loaded_plugin->config_node = config_node;
                    loaded_plugin->init_result = ret;

                    chain_append_key_value(shell->loaded_plugins, children[i].key, loaded_plugin);
                }
                close_yaml_parser(yaml_parser_wrapper_2);
}


/*function start_shell*/
void *start_shell(void *arg)
{
    shell_t *shell = (shell_t *)arg;
    chain_node_t *node=NULL;
    char command[MAX_COMMAND_LENGTH];
    char * cmd_ptr;
    char *command_args[MAX_COMMAND_ARGS];
    char *command_arg;
    int command_argc;

    void *current_plugin=NULL;

    char *help_message ="help: print the help message\n"
                        "exit: exit the shell\n"
                        "list: list all the plugins\n"
                        "load_plugin_from_file <plugin_conf.yaml>: load the plugin\n"
                        "unload <plugin_name>: unload the plugin\n"
                        "<plugin_name> <command>: execute the command of the plugin\n";

    char *sub_message = "end: back to the command root \n";

    FILE *fp = fdopen(shell->client_socket_fd, "w");
    if (fp == NULL){
        fprintf(stderr, "Failed to open client_socket_fd\n");
        return NULL;
    }
    // read the command from the socket
    while (1)
    {
        memset(command, 0, sizeof(command));
        memset(command_args, 0, sizeof(*command_args));
        command_argc = 0;

        fprintf(fp, ">>");
        fflush(fp);

        // read the command from the socket
        if (read(shell->client_socket_fd, command, MAX_COMMAND_LENGTH) < 0)
        {
            fprintf(stderr, "Failed to read command\n");
            break;
        }

        cmd_ptr = trim(command);
        // parse the command
        command_arg = strtok(cmd_ptr, " ");
        while (command_arg != NULL)
        {
            command_args[command_argc] = command_arg;
            command_argc++;
            command_arg = strtok(NULL, " ");
        }

        if (command_argc ==0)
        {
            continue;
        }

        
        if (current_plugin==NULL){
            // execute the command
            if (strcmp(command_args[0], "exit") == 0)
            {
                break;
            }else if (strcmp(command_args[0], "list") == 0){
                print_chain(shell->loaded_plugins, fp, NULL);
                continue;
            }else if (strcmp(command_args[0], "load_plugin_from_file") == 0){
                load_plugin_from_shell(shell, command_args[1]);
                continue;
            }else if (strcmp(command_args[0], "unload") == 0){
                //if the plugin is not loaded, print the error message
                node = chain_has_key(shell->loaded_plugins, command_args[1]);
                if (node==NULL){
                    fprintf(fp, "The plugin %s is not loaded\n", command_args[0]);
                    continue;
                }
                close_plugins_in_chain(shell->loaded_plugins, command_args[1]);
                continue;
            }else if ((node =chain_has_key(shell->loaded_plugins, command_args[0]))!=NULL){
                //check if the plungin have execute command interface.
                current_plugin = node;
                if (command_argc==2) {
                  execute_command_in_plugin(shell, (loaded_plugins_t*)node->value, &command_args[1], fp);
                }
                fprintf(fp, "%s", command_args[0]);
                continue;
            }else{
                // print the help message
                if (write(shell->client_socket_fd, help_message, strlen(help_message)) < 0)
                {
                    fprintf(stderr, "Failed to write message\n");
                    break;
                }
            }
        }else{

            if (strcmp(command_args[0], "end") == 0){
                current_plugin=NULL;
                continue;
            }
            //check if the plungin have execute command interface.
            chain_node_t * current_plugin_node = (chain_node_t *) current_plugin;
            if (chain_has_key(shell->loaded_plugins, current_plugin_node->key)){
                //execute the command
                execute_command_in_plugin(shell, (loaded_plugins_t*)current_plugin_node->value, &command_args[0], fp);
                if (strcmp(command_args[0], "help") == 0 )
                {
                    if (write(shell->client_socket_fd, sub_message, strlen(sub_message)) < 0){
                        fprintf(stderr, "Failed to write message\n");
                        break;
                    }
                }
                fprintf(fp, "%s", current_plugin_node->key);
            }else{
                current_plugin=NULL;
                fprintf(stderr, "Plugin not exists\n");
            }
        }
    }
    fclose(fp);
    return NULL;
}


/* Start to accept socket connection from remote clients
* It will create a new thread for each connection.
* The thread will execute the command and send the result back to the client
* The thread will be terminated when the client close the connection
* The function will return 0 if it is executed successfully, otherwise it will return -1
*/
void* waiting_shell_connection(void *arg)
{
    shell_t* root_shell = (shell_t*)arg;
    socklen_t client_addr_len = sizeof(root_shell->client_addr);

    while (1)
    {
        shell_t *shell = (shell_t *)malloc(sizeof(shell_t));
        memcpy(shell, root_shell, sizeof(shell_t));
        shell->socket = -1;
        shell->thread = 0;
        sem_post(&root_shell->sem);

        shell->client_socket_fd = accept(root_shell->socket, (struct sockaddr *)&shell->client_addr, &client_addr_len);
        if (shell->client_socket_fd <= 0)
        {
            fprintf(stderr, "Failed to accept socket\n");
            free(shell);
            return NULL;
        }

        char s[10] = {0};
        sprintf(s, "%d", shell->client_socket_fd);
        chain_append_key_value(shell->shells, s, shell);

        // create a new thread to handle the connection
        if (pthread_create(&shell->thread, NULL, start_shell, shell) != 0)
        {
            fprintf(stderr, "Failed to create thread\n");
            return NULL;
        }
    }

    return NULL;
}

void* init(yaml_parser_wrapper_t* yaml_parser_wrapper, yaml_node_t *config_node, yaml_node_t *global_config, chain_t *loaded_plugins){
    shell_t *shell = (shell_t *)malloc(sizeof(shell_t));
    memset(shell, 0, sizeof(shell_t));
    shell->yaml_parser_wrapper = yaml_parser_wrapper;
    shell->config_node = config_node;
    shell->global_config = global_config;
    shell->loaded_plugins = loaded_plugins;
    shell->socket = -1;
    shell->client_socket_fd = -1;
    shell->shells = new_chain();

    yaml_node_t * root_node = get_yaml_root_node(yaml_parser_wrapper);
    yaml_node_pair_t * pair = get_yaml_node_id_by_key(yaml_parser_wrapper, root_node, "plugins");
    shell->plugins_node_id = pair->value;

    // get the port number from the config file
    yaml_node_t *port_node = get_yaml_node_by_key(yaml_parser_wrapper, config_node, "port");
    if (port_node != NULL)
    {
        shell->port = atoi((char*)port_node->data.scalar.value);
    }

    shell->socket = open_socket(shell->port);
    if (shell->socket < 0)
    {
        fprintf(stderr, "Failed to open socket\n");
        free(shell);
        return NULL;
    }

    sem_init(&shell->sem, 0, 0);
    // start the shell thread
    if (pthread_create(&shell->thread, NULL, waiting_shell_connection, shell) != 0)
    {
        fprintf(stderr, "Failed to create thread\n");
        free(shell);
        sem_destroy(&shell->sem);
        return NULL;
    }

    sem_wait(&shell->sem);
    sem_destroy(&shell->sem);
    
    return shell;
}


void close_thread(void* node, FILE* fp,...){
    (void)fp;
    chain_node_t *_node = (chain_node_t *)node;
    shell_t *shell = (shell_t *)_node->value;

    if (shell->client_socket_fd >= 0)
    {
        shutdown(shell->socket, SHUT_RDWR);
        close(shell->client_socket_fd);
    }
    if (shell->thread>0) pthread_join(shell->thread, NULL);
}

void close_so(shell_t *shell){
    print_chain(shell->shells, stdout,close_thread);

    if (shell->socket >= 0)
    {
        shutdown(shell->socket, SHUT_RDWR);
        close(shell->socket);
        if (shell->thread>0) pthread_join(shell->thread, NULL);
    }
    free_chain(shell->shells, free);
    free(shell);
}