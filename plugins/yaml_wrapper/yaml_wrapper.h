#ifndef __YAML_WRAPPER__
#define __YAML_WRAPPER__

#include <yaml.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct yaml_parser_wrapper_s {
    FILE *fp;
    yaml_parser_t parser;
    yaml_document_t document;
} yaml_parser_wrapper_t;

typedef struct yaml_mapping_node_child_s {
    char *key;
    yaml_node_t *value;
} yaml_mapping_node_child_t;

/*declare all functions implemented in yaml_wrapper.c*/
yaml_parser_wrapper_t * init_yaml_parser(FILE *file);
void close_yaml_parser(yaml_parser_wrapper_t *yaml_parser_wrapper);
yaml_node_t * get_yaml_node_by_key(yaml_parser_wrapper_t *yaml_parser_wrapper, yaml_node_t *father_node, char *key);
yaml_mapping_node_child_t * get_yaml_mapping_node_children(yaml_parser_wrapper_t *yaml_parser_wrapper, yaml_node_t *father_node);
yaml_node_t * get_yaml_root_node(yaml_parser_wrapper_t *yaml_parser_wrapper);
yaml_node_t * get_yaml_node_by_path(yaml_parser_wrapper_t *yaml_parser_wrapper, char *path);
yaml_node_t * get_yaml_node_by_path_from(yaml_parser_wrapper_t *yaml_parser_wrapper, yaml_node_t *father_node, char *path);
yaml_node_pair_t * get_yaml_node_id_by_key(yaml_parser_wrapper_t *yaml_parser_wrapper, yaml_node_t *father_node, char *key);


int yaml_document_add_node(yaml_document_t *document, yaml_char_t *tag, yaml_node_type_t type,...);

void print_yaml_node(yaml_parser_wrapper_t *yaml_parser_wrapper, yaml_node_t *node, FILE *file);
void print_yaml_node_with_format(yaml_parser_wrapper_t *yaml_parser_wrapper, yaml_node_t *node, FILE *file, int indent,int) ;

int add_scalar_key_value_2_mapping_node(yaml_parser_wrapper_t *yaml_parser_wrapper, int father, char* key, char* value);
int create_map_2_mapping_node(yaml_parser_wrapper_t *yaml_parser_wrapper, int father, char* child_map_node_key, char*key, char* value);

void merge_from_file(yaml_parser_wrapper_t *yaml_parser_wrapper1, int node1_index, yaml_parser_wrapper_t *yaml_parser_wrapper2, int node2_index);


#ifdef __cplusplus
}
#endif

#endif
