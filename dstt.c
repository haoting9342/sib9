#include "chain_utils.h"
#include "dstt.h"
#include <stdio.h>
#include "plugins/yaml_wrapper/yaml_wrapper.h"
#include "plugin_handler.h"
#include <signal.h>
#include<unistd.h>

volatile sig_atomic_t flag = 0;

void sigint_handler(int sig) {
    flag = sig;
    flag = 1;
}


int main(int argc, char *argv[])
{
    char *filename = "config.yml";
    if (argc > 1)
    {
        filename = argv[1];
    }

    // open the config file
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        fprintf(stderr, "Failed to open file %s\n", filename);
        return 1;
    }

    // initialize the yaml parser
    yaml_parser_wrapper_t *yaml_parser_wrapper = init_yaml_parser(file);
    if (!yaml_parser_wrapper)
    {
        fprintf(stderr, "Failed to initialize yaml parser\n");
        return 1;
    }
    fclose(file);



    // Set up the signal handler
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    // get the root node of the yaml document
    yaml_node_t *root_node = get_yaml_root_node(yaml_parser_wrapper);
    if (!root_node)
    {
        fprintf(stderr, "Failed to get root node\n");
        return 1;
    }
    // print_yaml_node_with_format(yaml_parser_wrapper, root_node, stdout,2,0);

    yaml_node_t *plugins_nodes = get_yaml_node_by_key(yaml_parser_wrapper, root_node, "plugins");
    if (!plugins_nodes)
    {
        fprintf(stderr, "Failed to get plugins node\n");
        return 1;
    }
   

    yaml_node_t * global_config = get_yaml_node_by_key(yaml_parser_wrapper, root_node, "globalSettings");
    chain_t * sorted_plugins_configs = get_sorted_plugins_configs(yaml_parser_wrapper, plugins_nodes);

    chain_t * uniq_sorted_plugins_configs = remove_duplicated_keys(sorted_plugins_configs);

    free_chain(sorted_plugins_configs,NULL);
    sorted_plugins_configs=NULL;

    //load plugins
    chain_node_t *node = uniq_sorted_plugins_configs->head;
    chain_t* loaded_plugins = new_chain();
    while (node != NULL)
    {
        if (node->value!=NULL){
            //print node address to stdout
            yaml_node_t * config_node = (yaml_node_t *) node->value;
            fprintf(stdout, "loading plugin %s ...\n", node->key);
            void *plugin = load_plugin(yaml_parser_wrapper, config_node);
            
            if (plugin ==NULL)  {
                break;
            }
            // initialize the plugin
            void* ret = init_plugin(plugin, yaml_parser_wrapper, config_node, global_config, loaded_plugins);
            if (ret==NULL)
            {
                fprintf(stderr, "Failed to initialize plugin %s\n", node->key);
                dlclose(plugin);
                break;
            }

            loaded_plugins_t *loaded_plugin = malloc(sizeof(loaded_plugins_t));
            loaded_plugin->plugin = plugin;
            loaded_plugin->config_node = config_node;
            loaded_plugin->init_result = ret;

            chain_append_key_value(loaded_plugins, node->key, loaded_plugin);

        }
        
        node = node->next;
    }

    while (!flag){
        sleep(1);
        
    }


    free_chain(uniq_sorted_plugins_configs,NULL);
    uniq_sorted_plugins_configs=NULL;

    close_plugins_in_chain(loaded_plugins, NULL);
    // print_chain(loaded_plugins,stdout,NULL);


    free_chain(loaded_plugins,NULL);
    loaded_plugins=NULL;

    //print a source code line and file name to stdout
    fprintf(stdout, "This is line %d of file %s\n", __LINE__, __FILE__);
    
    //free the yaml parser
    close_yaml_parser(yaml_parser_wrapper);
    // fclose(file);

    return 0;

}


