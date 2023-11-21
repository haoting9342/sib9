#include "chain_utils.h"

void print_node_address_info(void* node, FILE* fp, ...){
    chain_node_t *_node = (chain_node_t *)node;
    fprintf(fp, "Node address: %p, ", _node);
    fprintf(fp, "Node->prev address: %p, ", _node->prev);
    fprintf(fp, "Node->next address: %p\n", _node->next);
}

