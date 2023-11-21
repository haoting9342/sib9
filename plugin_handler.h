#ifndef __PLUGIN_HANDLER__
#define __PLUGIN_HANDLER__

#include "plugins/yaml_wrapper/yaml_wrapper.h"
#include "chain.h"
#include <dlfcn.h>

typedef struct loaded_plugins_s {
    void *plugin;
    void *init_result;
    yaml_node_t *config_node;
} loaded_plugins_t;

/*Below will be a block of functions declarations */
void* load_plugin(yaml_parser_wrapper_t *yaml_parser_wrapper, yaml_node_t *node);
void *load_plugin_from_path(char *path);
void* init_plugin(void *plugin, yaml_parser_wrapper_t* yaml_parser_wrapper, yaml_node_t *config_node, yaml_node_t * global_config, chain_t *chain);
chain_t * get_sorted_plugins_configs (yaml_parser_wrapper_t *yaml_parser_wrapper, yaml_node_t *plugins_node);
/*declaration of close plugin*/
int close_plugin(void *plugin);
void close_plugins_in_chain(chain_t *chain, char *plugin_name);

#endif
