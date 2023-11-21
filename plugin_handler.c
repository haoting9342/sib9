#include <stdio.h>
#include "plugin_handler.h"
#include "chain.h"
#include "chain_utils.h"
/* 
* Load a c library plugin from a yaml node first, after loading the plugin, initialize the plugin with the yaml node. 
*/
void* load_plugin(yaml_parser_wrapper_t *yaml_parser_wrapper, yaml_node_t *node)
{
    //check if configuration is disabled.
    yaml_node_t *disabled_node = get_yaml_node_by_key(yaml_parser_wrapper, node, "disabled");
    if (disabled_node)
    {
        if (disabled_node->type == YAML_SCALAR_NODE)
        {
            if (strcmp((char *)disabled_node->data.scalar.value, "true") == 0)
            {
                fprintf(stderr, "Plugin is disabled\n");
                return NULL;
            }
        }
    }
    // get the plugin path
    yaml_node_t *path_node = get_yaml_node_by_key(yaml_parser_wrapper, node, "path");
    if (!path_node)
    {
        fprintf(stderr, "Failed to get plugin path\n");
        return NULL;
    }
    char *path = (char *)path_node->data.scalar.value;

    // load the plugin
    void *plugin = load_plugin_from_path(path);

    return plugin;
}

/* load_plugin_from_path.
* Load a c library plugin from a path.
*/
void *load_plugin_from_path(char *path)
{
    char *error; 
    void *plugin = dlopen(path, RTLD_LOCAL | RTLD_LAZY);
    if (!plugin)
    {
        error = dlerror(); // Get the error message from dlerror
        fprintf(stderr, "dlopen error: %s\n", error); // Print the error message to stderr
        return NULL;
    }
    return plugin;
}


/* init_plugin.
* Initialize a plugin with a yaml node. The init function is defined in the plugin with symble "init". It accepts a yaml config node and yaml document  as argument.
* return 0 if success, 1 if failed.
*/
void* init_plugin(void *plugin, yaml_parser_wrapper_t* yaml_parser_wrapper, yaml_node_t *config_node,  yaml_node_t * global_config, chain_t *loaded_plugins)
{
    char *error;
    void* (*init)(yaml_parser_wrapper_t* , yaml_node_t *, yaml_node_t *, chain_t *) = dlsym(plugin, "init");
    if (!init)
    {
        error = dlerror(); // Get the error message from dlerror
        fprintf(stderr, "find function error: %s\n", error); // Print the error message to stderr
        return NULL;
    }
    void* ret = init(yaml_parser_wrapper, config_node, global_config, loaded_plugins);
    if (ret==NULL)
    {
        fprintf(stderr, "Failed to initialize plugin\n");
        return NULL;
    }
    return ret;
}

/* close_plugin.
* Close a plugin. The close function is defined in the plugin with symble "close". It accepts no argument.
* return 0 if success, 1 if failed.
*/
int close_plugin(void *loaded_plugin)
{
    loaded_plugins_t * p = (loaded_plugins_t *)loaded_plugin;
    void *plugin = p->plugin;
    if (!plugin) return 0;

    void * close = dlsym(plugin, "close_so");
    if (close) {
        ((void (*)(void *))close)(p->init_result);
    }

    //close plugin 
    return dlclose(plugin);
}


/*
* Fucntion get_sorted_plugins_configs is used to sort the plugins' by their dependencies list configurations in the yaml node.
* Each dependency is a filename if it is in the default system searching path. Otherwise, the filename shall 
* includde absolute path or relative path to program runnning path.  The ordering is based on the dependencies list.
* A plugin shall behind of the plugins they depend on.
* Input:  
*      yaml_parser_wrapper_t *yaml_parser_wrapper, 
*      yaml_node_t *node.   "plugins" yaml nodes which children is list of yaml nodes for different plugins.
* Output:
*      yaml_node_t *node.  The sorted plugins' yaml nodes.
*/

chain_t * _get_sorted_plugins_configs (yaml_parser_wrapper_t *yaml_parser_wrapper, chain_t *level_0_chain){
    chain_node_t *node = level_0_chain->head;
    fprintf(stdout, "node address is %p\n", node);
    print_chain(level_0_chain,stdout,print_node_address_info);

    while (node != NULL){
        yaml_node_t *dependencies_node = get_yaml_node_by_key(yaml_parser_wrapper, (yaml_node_t *)node->value, "dependency");
        
        if (dependencies_node){
            for (int j = 0; j < dependencies_node->data.sequence.items.top - dependencies_node->data.sequence.items.start; j++) {
                yaml_node_t *dependency_node = yaml_document_get_node(&yaml_parser_wrapper->document, dependencies_node->data.sequence.items.start[j]);
                char *dependency = (char *)dependency_node->data.scalar.value;
                chain_node_t *p = chain_has_key(level_0_chain, dependency); 

                if (p != NULL&&p!=node){
                    chain_move_chain_node_to_before(level_0_chain, node, p);
                }
            }
        }
        node = node->next;
    } 

    return level_0_chain;
}


chain_t * get_sorted_plugins_configs (yaml_parser_wrapper_t *yaml_parser_wrapper, yaml_node_t *plugins_node)
{
    chain_t* level_0_chain = new_chain();

    yaml_mapping_node_child_t * children = get_yaml_mapping_node_children(yaml_parser_wrapper, plugins_node);
    for (int i = 0; i < plugins_node->data.mapping.pairs.top - plugins_node->data.mapping.pairs.start; i++) {
        chain_append_key_value(level_0_chain, children[i].key, children[i].value);
    }
    free(children);

    print_chain(level_0_chain,stdout,NULL);
    return _get_sorted_plugins_configs(yaml_parser_wrapper, level_0_chain);
}

void close_plugins_in_chain(chain_t *chain, char *plugin_name)
{
    chain_node_t *node = chain->tail;

    while (node)
    {
        if (!plugin_name || strcmp(node->key, plugin_name) == 0){
            chain_remove_node(chain, node);
            loaded_plugins_t *loaded_plugin = (loaded_plugins_t *)node->value;
            fprintf(stderr, "Closing plugin %s\n", node->key);
            close_plugin(loaded_plugin);
            free(node->key);
            free(loaded_plugin);
            free(node);
        }
        node = node->prev;
    }
}