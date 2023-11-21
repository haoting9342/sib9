#include "chain.h"


void free_chain(chain_t* ct, void (*free_func)(void*)) {
    if (ct == NULL) {
        return;
    }
    pthread_mutex_lock(&ct->mutex);
    chain_node_t *node = ct->head;
    while (node != NULL) {
        chain_node_t *next = node->next;
        if (free_func) free_func(node->value);
        free(node->key);
        free(node);
        node = next;
    }    
    pthread_mutex_unlock(&ct->mutex);
    free(ct);
}


chain_node_t* chain_append_key_value(chain_t* ct, char* key, void * value){
    if (ct == NULL) {
        return NULL;
    }
    
    chain_node_t *node = (chain_node_t*)malloc(sizeof(chain_node_t));
    memset(node, 0, sizeof(chain_node_t));
    node->key = strdup(key);
    node->value = value;
    pthread_mutex_lock(&ct->mutex);

    if (ct->head == NULL) {
        ct->head = node;
        ct->tail = node;
    }else{
        ct->tail->next = node;
        node->prev = ct->tail;
        ct->tail = node;
    } 

    ct->size++;
    pthread_mutex_unlock(&ct->mutex);
    return node;
}

chain_node_t* chain_remove_by_key(chain_t* ct, char* key){
    if (ct == NULL) {
        return NULL;
    }
    pthread_mutex_lock(&ct->mutex);
    chain_node_t *node = ct->head;
    chain_node_t *ret = NULL;
    while (node != NULL) {
        if (strcmp(node->key, key) == 0) {

            if (node->prev != NULL) {
                node->prev->next = node->next;
            }
            if (node->next != NULL) {
                node->next->prev = node->prev;
            }
            if (ct->head == node) {
                ct->head = node->next;
            }
            if (ct->tail == node) {
                ct->tail = node->prev;
            }
            ct->size--;
            ret = node;
        }
        node = node->next;
    }
    pthread_mutex_unlock(&ct->mutex);
    return ret;
}

chain_node_t* chain_insert_key_value_before_node(chain_t* ct, chain_node_t* pos, char* key, void * value){
    if (ct == NULL || pos==NULL) {
        return NULL;
    }
    pthread_mutex_lock(&ct->mutex);
    chain_node_t *node = (chain_node_t*)malloc(sizeof(chain_node_t));
    memset(node, 0, sizeof(chain_node_t));
    node->key = strdup(key);
    node->value = value;
    node->next = pos;

    node->prev = pos->prev;
    if (pos->prev != NULL) {
        pos->prev->next = node;
    }
    pos->prev = node;
    if (ct->head == pos) {
        ct->head = node;
    }
    ct->size++;
    pthread_mutex_unlock(&ct->mutex);
    return node;
}

chain_node_t* chain_insert_key_value_after_node(chain_t* ct, chain_node_t* pos, char* key, void * value){
    if (ct == NULL) {
        return NULL;
    }
    pthread_mutex_lock(&ct->mutex);
    chain_node_t *node = (chain_node_t*)malloc(sizeof(chain_node_t));
    memset(node, 0, sizeof(chain_node_t));
    node->key = strdup(key);
    node->value = value;
    node->next = pos->next;
    node->prev = pos;
    if (pos->next != NULL) {
        pos->next->prev = node;
    }
    pos->next = node;
    if (ct->tail == pos) {
        ct->tail = node;
    }
    ct->size++;
    pthread_mutex_unlock(&ct->mutex);
    return node;
}


chain_node_t* chain_has_key(chain_t* ct, char* key){
    if (ct == NULL) {
        return NULL;
    }
    pthread_mutex_lock(&ct->mutex);
    chain_node_t *node = ct->head;
    while (node != NULL) {
        if (strcmp(node->key, key) == 0) {
            pthread_mutex_unlock(&ct->mutex);
            return node;
        }
        node = node->next;
    }
    pthread_mutex_unlock(&ct->mutex);
    return NULL;
}

chain_t* remove_duplicated_keys(chain_t* ct){
    if (ct == NULL) {
        return NULL;
    }
    chain_t *ret = new_chain();
    pthread_mutex_lock(&ct->mutex);
    chain_node_t *node = ct->head;
    while (node != NULL) {
        if (chain_has_key(ret, node->key) == NULL) {
            chain_append_key_value(ret, node->key, node->value);
        }
        node = node->next;
    }
    pthread_mutex_unlock(&ct->mutex);
    return ret;
}

chain_t * new_chain(){
    chain_t *ct = (chain_t*)malloc(sizeof(chain_t));
    memset(ct, 0, sizeof(chain_t));
    ct->head = NULL;
    ct->tail = NULL;
    pthread_mutex_init(&ct->mutex, NULL);
    return ct;
}

chain_node_t* chain_remove_node(chain_t* ct, chain_node_t* node){
    if (ct == NULL) {
        return NULL;
    }
    pthread_mutex_lock(&ct->mutex);
    if (node->prev != NULL) {
        node->prev->next = node->next;
    }
    if (node->next != NULL) {
        node->next->prev = node->prev;
    }
    if (ct->head == node) {
        ct->head = node->next;
    }
    if (ct->tail == node) {
        ct->tail = node->prev;
    }
    ct->size--;
    pthread_mutex_unlock(&ct->mutex);
    return node;
}

chain_node_t* chain_move_chain_node_to_before(chain_t* ct, chain_node_t* ref_node, chain_node_t* moving_node){
    if (ct == NULL || ref_node == NULL || moving_node == NULL) {
        return NULL;
    }
    
    pthread_mutex_lock(&ct->mutex);
    if (moving_node->prev != NULL) {
        moving_node->prev->next = moving_node->next;
    }
    if (moving_node->next != NULL) {
        moving_node->next->prev = moving_node->prev;
    }
    if (ct->head == moving_node) {
        ct->head = moving_node->next;
    }
    if (ct->tail == moving_node) {
        ct->tail = moving_node->prev;
    }
    moving_node->next = ref_node;
    moving_node->prev = ref_node->prev;
    if (ref_node->prev != NULL) {
        ref_node->prev->next = moving_node;
    }
    ref_node->prev = moving_node;
    if (ct->head == ref_node) {
        ct->head = moving_node;
    }
    pthread_mutex_unlock(&ct->mutex);
    return moving_node;
}


chain_t* combine_chain(chain_t* ct1, chain_t* ct2){
    if (ct1 == NULL) {
        return ct2;
    }
    if (ct2 == NULL) {
        return ct1;
    }
    pthread_mutex_lock(&ct1->mutex);
    pthread_mutex_lock(&ct2->mutex);
    ct1->tail->next = ct2->head;
    ct2->head->prev = ct1->tail;
    ct1->tail = ct2->tail;
    ct1->size += ct2->size;
    pthread_mutex_unlock(&ct2->mutex);
    pthread_mutex_unlock(&ct1->mutex);
    free(ct2);
    return ct1;
}



void print_chain(chain_t * ct, FILE *fp, void (*print_func)(void*, FILE*, ...)){
    if (ct == NULL) {
        return;
    }

    pthread_mutex_lock(&ct->mutex);
    chain_node_t *node = ct->head;
    while (node != NULL) {
        fprintf(fp, "%s ", node->key);
        if (print_func) print_func(node, fp);
        node = node->next;
    }
    pthread_mutex_unlock(&ct->mutex);
    fprintf(fp, " \n");
    
}


//put a key value pair into chain head.  the key is not duplicated and generated as from uuid
chain_node_t* queue_push(chain_t* ct, void * value){
    if (ct == NULL) {
        return NULL;
    }
    uuid_t uuid;
    chain_node_t *node = (chain_node_t*)malloc(sizeof(chain_node_t));
    memset(node, 0, sizeof(chain_node_t));
    node->value = value;
    uuid_generate_random(uuid);
    node->key = (char *)uuid;
    pthread_mutex_lock(&ct->mutex);
    node->next = ct->head;
    if (ct->head != NULL) {
        ct->head->prev = node;
    }
    ct->head = node;
    if (ct->tail == NULL) {
        ct->tail = node;
    }
    ct->size++;
    pthread_cond_signal(&ct->cond);
    pthread_mutex_unlock(&ct->mutex);
    return node;

}


//pop from chain tail and return the value
//if cat is tempty, then the call will blocked until a value is available
chain_node_t* queue_get(chain_t* ct){
    if (ct == NULL) {
        return NULL;
    }
    pthread_mutex_lock(&ct->mutex);
    while (ct->tail == NULL) {
        pthread_cond_wait(&ct->cond, &ct->mutex);
    }
    chain_node_t *node = ct->tail;
    ct->tail = node->prev;
    if (ct->tail == NULL) {
        ct->head = NULL;
    }
    ct->size--;
    pthread_mutex_unlock(&ct->mutex);
    return node;
}