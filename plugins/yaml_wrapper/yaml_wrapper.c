#include <stdio.h>
#include "yaml_wrapper.h"


yaml_parser_wrapper_t * init_yaml_parser(FILE *file) {
    yaml_parser_wrapper_t *yaml_parser_wrapper = malloc(sizeof(struct yaml_parser_wrapper_s));
    memset(yaml_parser_wrapper, 0, sizeof(struct yaml_parser_wrapper_s));


    // Initialize the parser
    if (!yaml_parser_initialize(&yaml_parser_wrapper->parser)) {
        fprintf(stderr, "Failed to initialize parser\n");
        return NULL;
    }

    // Set the input file
    yaml_parser_set_input_file(&yaml_parser_wrapper->parser, file);

    // Parse the YAML stream into a document
    if (!yaml_parser_load(&yaml_parser_wrapper->parser, &yaml_parser_wrapper->document)) {
        fprintf(stderr, "Failed to parse document\n");
        return NULL;
    }

    yaml_parser_wrapper->fp = file;
    return yaml_parser_wrapper;
}

void close_yaml_parser(yaml_parser_wrapper_t *yaml_parser_wrapper) {
    yaml_parser_delete(&yaml_parser_wrapper->parser);
    yaml_document_delete(&yaml_parser_wrapper->document);
    yaml_parser_wrapper->fp = NULL;
    free(yaml_parser_wrapper);
}

yaml_node_pair_t * get_yaml_node_id_by_key(yaml_parser_wrapper_t *yaml_parser_wrapper, yaml_node_t *father_node, char *key) {
    yaml_node_t *node = father_node;
    if (node->type != YAML_MAPPING_NODE) {
        fprintf(stderr, "Error: node is not a mapping node\n");
        return NULL;
    }
    yaml_node_pair_t *pair = node->data.mapping.pairs.start;
    for (int i = 0; i < node->data.mapping.pairs.top - node->data.mapping.pairs.start; i++) {
        yaml_node_t *key_node = yaml_document_get_node(&yaml_parser_wrapper->document, pair[i].key);
        if (!key_node){
            fprintf(stderr, "Error: key node is NULL\n");
            return NULL;
        }
        if (key_node->type != YAML_SCALAR_NODE) {
            fprintf(stderr, "Error: key node %s is not a scalar node\n", key);
            return NULL;
        }
        if (strcmp((char *)key_node->data.scalar.value, key) == 0) {
            return &pair[i];
        }
    }
    return NULL;

}

yaml_node_t * get_yaml_node_by_key(yaml_parser_wrapper_t *yaml_parser_wrapper, yaml_node_t *father_node, char *key) {
    if (father_node->type != YAML_MAPPING_NODE) {
        fprintf(stderr, "Error: node is not a mapping node\n");
        return NULL;
    }
    yaml_node_pair_t *pair = get_yaml_node_id_by_key(yaml_parser_wrapper, father_node, key);
    if (!pair) {
        fprintf(stderr, "Error: key \"%s\" not found\n", key);
        return NULL;
    }
    return yaml_document_get_node(&yaml_parser_wrapper->document, pair->value);
}

yaml_mapping_node_child_t * get_yaml_mapping_node_children(yaml_parser_wrapper_t *yaml_parser_wrapper, yaml_node_t *node) {
    if (node->type != YAML_MAPPING_NODE) {
        fprintf(stderr, "Error: node is not a mapping node\n");
        return NULL;
    }
    yaml_mapping_node_child_t *children = malloc(sizeof(struct yaml_mapping_node_child_s) * (node->data.mapping.pairs.top - node->data.mapping.pairs.start));
    yaml_node_pair_t *pair = node->data.mapping.pairs.start;
    for (int i = 0; i < node->data.mapping.pairs.top - node->data.mapping.pairs.start; i++) {
        yaml_node_t *key_node = yaml_document_get_node(&yaml_parser_wrapper->document, pair[i].key);
        if (key_node->type != YAML_SCALAR_NODE) {
            fprintf(stderr, "Error: key node is not a scalar node\n");
            return NULL;
        }
        children[i].key =(char *) key_node->data.scalar.value;
        yaml_node_t *value_node = yaml_document_get_node(&yaml_parser_wrapper->document, pair[i].value);
        children[i].value = value_node;
    }
    return children;
}

 // Get the root node of the document
yaml_node_t * get_yaml_root_node(yaml_parser_wrapper_t *yaml_parser_wrapper) {
    yaml_node_t *node = yaml_document_get_root_node(&yaml_parser_wrapper->document);
     // Check that the root node is a mapping node
    if (node->type != YAML_MAPPING_NODE) {
        fprintf(stderr, "Expected mapping node\n");
        return NULL;
    }
    return node;
}

//implementation of get_yaml_node_by_path
yaml_node_t * get_yaml_node_by_path(yaml_parser_wrapper_t *yaml_parser_wrapper, char *path) {
    yaml_node_t *node = get_yaml_root_node(yaml_parser_wrapper);
    char *token = strtok(path, ".");
    while (token != NULL) {
        node = get_yaml_node_by_key(yaml_parser_wrapper, node, token);
        if (node == NULL) {
            return NULL;
        }
        token = strtok(NULL, ".");
    }
    return node;
}



/*print out a yaml_node_t to a FILE such as stdout*/
void print_yaml_node(yaml_parser_wrapper_t *yaml_parser_wrapper, yaml_node_t *node, FILE *file) {
    switch (node->type) {
        case YAML_NO_NODE:
            fprintf(file, "YAML_NO_NODE\n");
            break;
        case YAML_SCALAR_NODE:
            fprintf(file, "YAML_SCALAR_NODE: %s\n", node->data.scalar.value);
            break;
        case YAML_SEQUENCE_NODE:
            fprintf(file, "YAML_SEQUENCE_NODE\n");
            for (int i = 0; i < node->data.sequence.items.top - node->data.sequence.items.start; i++) {
                yaml_node_t *item = yaml_document_get_node(&yaml_parser_wrapper->document, node->data.sequence.items.start[i]);
                print_yaml_node(yaml_parser_wrapper, item, file);
            }
            break;
        case YAML_MAPPING_NODE:
            fprintf(file, "YAML_MAPPING_NODE\n");
            yaml_mapping_node_child_t *children = get_yaml_mapping_node_children(yaml_parser_wrapper, node);
            for (int i = 0; i < node->data.mapping.pairs.top - node->data.mapping.pairs.start; i++) {
                fprintf(file, "%s: ", children[i].key);
                print_yaml_node(yaml_parser_wrapper, children[i].value, file);
            }
            free(children);
            break;
    }
}


void print_indent(FILE *file, int indent) {
    if (indent>0) fprintf(file, "\n");
    for (int i = 0; i < indent; i++) {
        fprintf(file, "  ");
    }
}

void print_yaml_node_with_format(yaml_parser_wrapper_t *yaml_parser_wrapper, yaml_node_t *node, FILE *file, int indent, int parent_is_list) {
    yaml_mapping_node_child_t *children;
    switch (node->type) {
        case YAML_NO_NODE:
            fprintf(file, "YAML_NO_NODE\n");
            break;
        case YAML_SCALAR_NODE:
            fprintf(file, "%s", node->data.scalar.value);
            break;
        case YAML_SEQUENCE_NODE:
            for (int i = 0; i < node->data.sequence.items.top - node->data.sequence.items.start; i++) {                
                if (!(i==0 && parent_is_list==1)){
                    print_indent(file,  indent);
                    fprintf(file, "- ");
                }
                
                yaml_node_t *item = yaml_document_get_node(&yaml_parser_wrapper->document, node->data.sequence.items.start[i]);
                print_yaml_node_with_format(yaml_parser_wrapper, item, file, indent+1, 1);
            }
            break;
        case YAML_MAPPING_NODE:
            children = get_yaml_mapping_node_children(yaml_parser_wrapper, node);
            for (int i = 0; i < node->data.mapping.pairs.top - node->data.mapping.pairs.start; i++) {
                print_indent(file, (i == 0 && parent_is_list == 1) ? 0 : indent);
                fprintf(file, "%s: ", children[i].key);
                print_yaml_node_with_format(yaml_parser_wrapper, children[i].value, file, indent + 1, 0);   
            }
            free(children);
            break;
    }
}

yaml_node_t * get_yaml_node_by_path_from(yaml_parser_wrapper_t *yaml_parser_wrapper, yaml_node_t *node, char *path){
    char *copy = strdup(path);
    char *token = strtok(copy, ".");
    yaml_node_t *next = node;
    
    while (token != NULL) {
        next = get_yaml_node_by_key(yaml_parser_wrapper, next, token);
        
        if (next == NULL) {
            break;
        }
        token = strtok(NULL, ".");
    }
    free(copy);
    return next;
}

int add_scalar_key_value_2_mapping_node(yaml_parser_wrapper_t *yaml_parser_wrapper, int father, char* key, char* value){
    yaml_document_t *document = &yaml_parser_wrapper->document;
    yaml_node_t *node = yaml_document_get_node(document, father);
    if (node->type != YAML_MAPPING_NODE) {
        fprintf(stderr, "Expected mapping node\n");
        return -1;
    }
    int key_node = yaml_document_add_scalar(document, NULL, (yaml_char_t *) key, -1, YAML_PLAIN_SCALAR_STYLE);
    int value_node = yaml_document_add_scalar(document, NULL, (yaml_char_t *) value, -1, YAML_PLAIN_SCALAR_STYLE);
    yaml_document_append_mapping_pair(document, father, key_node, value_node);
    return key_node;

}


int create_map_2_mapping_node(yaml_parser_wrapper_t *yaml_parser_wrapper, int father, char* child_map_node_key, char*key, char* value){
    yaml_document_t *document = &yaml_parser_wrapper->document;
    yaml_node_t *node = yaml_document_get_node(document, father);
    if (node->type != YAML_MAPPING_NODE) {
        fprintf(stderr, "Expected mapping node\n");
        return -1;
    }
    int child_map_node = yaml_document_add_mapping(document, NULL, YAML_BLOCK_MAPPING_STYLE);
    add_scalar_key_value_2_mapping_node(yaml_parser_wrapper, child_map_node, key, value);
    int child_map_node_key_node = yaml_document_add_scalar(document, NULL, (yaml_char_t *) child_map_node_key, -1, YAML_PLAIN_SCALAR_STYLE);
    yaml_document_append_mapping_pair(document, father, child_map_node_key_node, child_map_node);
    return child_map_node;
}


int yaml_document_add_node(yaml_document_t *document, yaml_char_t *tag, yaml_node_type_t type,...){
    switch (type) {
        case YAML_NO_NODE:
            return yaml_document_add_scalar(document, tag, (yaml_char_t *)"", -1, YAML_PLAIN_SCALAR_STYLE);
            break;
        case YAML_SCALAR_NODE:
            return yaml_document_add_scalar(document, tag, (yaml_char_t *)"", -1, YAML_PLAIN_SCALAR_STYLE);
            break;
        case YAML_SEQUENCE_NODE:
            return yaml_document_add_sequence(document, tag, YAML_BLOCK_SEQUENCE_STYLE);
            break;
        case YAML_MAPPING_NODE:
            return yaml_document_add_mapping(document, tag, YAML_BLOCK_MAPPING_STYLE);
            break;
    }
    return 0;
}

// replace update node2 content into node1, return the updated node1.
void merge_from_file(yaml_parser_wrapper_t *yaml_parser_wrapper1, int node1_index, 
                              yaml_parser_wrapper_t *yaml_parser_wrapper2, int node2_index){
    yaml_document_t *document1 = &yaml_parser_wrapper1->document;
    yaml_document_t *document2 = &yaml_parser_wrapper2->document;
    yaml_node_t *node1 = yaml_document_get_node(document1, node1_index);
    yaml_node_t *node2 = yaml_document_get_node(document2, node2_index);
    if (node1->type == node2->type) {
        switch (node1->type) {
            case YAML_NO_NODE:
                return ;
                break;
            case YAML_SCALAR_NODE:
                node1->data.scalar.value = node2->data.scalar.value;
                return ;
                break;
            case YAML_SEQUENCE_NODE:
                for (int i = 0; i < node2->data.sequence.items.top - node2->data.sequence.items.start; i++) {
                    yaml_node_t *item = yaml_document_get_node(document2, node2->data.sequence.items.start[i]);
                    int new_item_index = yaml_document_add_node(document1, NULL, item->type);
                    merge_from_file(yaml_parser_wrapper1, new_item_index, yaml_parser_wrapper2, node2->data.sequence.items.start[i]);
                    yaml_document_append_sequence_item(document1, node1_index, new_item_index);
                    // Need to remove duplicate items, but how???????????????
                }
                break;
            case YAML_MAPPING_NODE:
                {
                    for (int i = 0; i < node2->data.mapping.pairs.top - node2->data.mapping.pairs.start; i++) {
                        yaml_node_pair_t * pair2 = &node2->data.mapping.pairs.start[i];
                        yaml_node_t *key_node2 = yaml_document_get_node(document2, pair2->key);
                        yaml_node_t *value_node2 = yaml_document_get_node(document2, pair2->value);
                        int is_new_item=1;
                        for ( int j = 0; j < node1->data.mapping.pairs.top - node1->data.mapping.pairs.start; j++) {
                            yaml_node_pair_t *pair = &node1->data.mapping.pairs.start[j];
                            yaml_node_t *key_node1 = yaml_document_get_node(document1, pair->key);
                            if (strcmp((char *)key_node1->data.scalar.value, (char *)key_node2->data.scalar.value) == 0) {
                                merge_from_file(yaml_parser_wrapper1, pair->value, yaml_parser_wrapper2, pair2->value);
                                is_new_item=0;
                                break;
                            }
                        }
                        if (is_new_item){
                            int new_key_node_index = yaml_document_add_node(document1, NULL, key_node2->type);
                            int new_value_node_index = yaml_document_add_node(document1, NULL, value_node2->type);
                            merge_from_file(yaml_parser_wrapper1, new_key_node_index, yaml_parser_wrapper2, pair2->key);
                            merge_from_file(yaml_parser_wrapper1, new_value_node_index, yaml_parser_wrapper2, pair2->value);
                            yaml_document_append_mapping_pair(document1, node1_index, new_key_node_index, new_value_node_index);
                        }
                    }
                    break;
                }
        }
    }
}
