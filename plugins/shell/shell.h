#ifndef __SHELL_H__
#define __SHELL_H__
// struct to store the shell

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include "../yaml_wrapper/yaml_wrapper.h"
#include "../../plugin_handler.h"
#include "../../chain.h"
#include "../../chain_utils.h"
#include <stdarg.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <ctype.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_COMMANDS 100
#define MAX_COMMAND_ARGS 10
#define MAX_COMMAND_ARG_LENGTH 100

#define MAX_SHELL_NUMBER 10


typedef struct shell_s
{   int port;
    int socket;
    int client_socket_fd;
    struct sockaddr_in client_addr;
    pthread_t thread;
    sem_t sem;
    chain_t *loaded_plugins;
    yaml_parser_wrapper_t* yaml_parser_wrapper;
    yaml_node_t *config_node;
    yaml_node_t *global_config;
    int plugins_node_id;
    chain_t *shells;
} shell_t;


char *ltrim(char *s)
{
    while(isspace(*s)) s++;
    return s;
}

char *rtrim(char *s)
{
    char* back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}

char *trim(char *s)
{
    return rtrim(ltrim(s)); 
}

#endif
