
#ifndef __M_CHAIN__
#define __M_CHAIN__

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdarg.h>
#include <uuid/uuid.h>

// define a structure for a key-value pair
typedef struct node_s {
  char* key; // the key
  void* value; // the value
  struct node_s* next;
  struct node_s* prev;  
} chain_node_t;

// define a structure for a map
typedef struct chain_s {
  chain_node_t* head;
  chain_node_t* tail;
  size_t size;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
} chain_t;

/*a block of functions declarations*/
chain_t * new_chain();
void free_chain(chain_t* ct, void (*free_func)(void*)) ;
chain_node_t* chain_append_key_value(chain_t* ct, char* key, void * value);
chain_node_t* chain_remove_by_key(chain_t* ct, char* key);
chain_node_t* chain_remove_node(chain_t* ct, chain_node_t* node);
chain_node_t* chain_insert_key_value_before_node(chain_t* ct, chain_node_t* pos, char* key, void * value);
chain_node_t* chain_insert_key_value_after_node(chain_t* ct, chain_node_t* pos, char* key, void * value);
chain_node_t* chain_move_chain_node_to_before(chain_t* ct, chain_node_t* ref_node, chain_node_t* moving_node);
chain_node_t* chain_has_key(chain_t* ct, char* key);

chain_t* combine_chain(chain_t* ct1, chain_t* ct2);
chain_t* remove_duplicated_keys(chain_t* ct);

void print_chain(chain_t * ct, FILE *fp, void (*print_func)(void*, FILE*, ...));

chain_node_t* queque_push(chain_t* ct, char* key, void * value);
chain_node_t* queue_get(chain_t* ct);


#endif
