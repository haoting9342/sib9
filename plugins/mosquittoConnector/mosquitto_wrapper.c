#include "../plugins.h"
#include "../../chain.h"
#include <stdio.h>
#include <uuid/uuid.h>
#include "mosquitto_wrapper.h"

static mosquitto_connector_config_t mosquitto_connector_config = {0};
static chain_t *mosquitto_connector_chain = NULL;

static void __destroy_mosquitto_connection(void *mc){
    mosquitto_connection_t *connector = (mosquitto_connection_t *)mc;
    mosquitto_loop_stop(connector->mosq, 1);
    mosquitto_disconnect(connector->mosq);
    mosquitto_destroy(connector->mosq);
    free(connector);
}

//implement mosquitto_connect_callback
static void __on_connect(struct mosquitto *mosq, void *obj, int rc){
    (void)mosq;
    (void)rc;
    mosquitto_connection_t *connector = (mosquitto_connection_t *)obj;
    if (rc){
        fprintf(stderr, "mosquitto connection failed\n");
        return;
    }
    fprintf(stderr, "mosquitto connection success\n");
    connector->connected = 1;
} 


int close_mosquitto_connection(mosquitto_connection_t *connector){
    chain_remove_by_key(mosquitto_connector_chain, (char *) connector->uuid);
    __destroy_mosquitto_connection(connector);
    return 0;
}

void close_so(void *ct){
    (void)ct;
    free_chain(mosquitto_connector_chain, __destroy_mosquitto_connection);
    mosquitto_lib_cleanup();
}

mosquitto_connection_t * new_mosquitto_connection(yaml_node_t *mosquitto_config_node){
    yaml_node_t *host_node = get_yaml_node_by_key(mosquitto_connector_config.yaml_parser_wrapper, mosquitto_config_node, "host");
    if (!host_node){
        fprintf(stderr, "host is not specified in mosquitto config\n");
        return NULL;
    }
    yaml_node_t *port_node = get_yaml_node_by_key(mosquitto_connector_config.yaml_parser_wrapper, mosquitto_config_node, "port");

    mosquitto_connection_t *connector = (mosquitto_connection_t *)malloc(sizeof(mosquitto_connection_t));
    memset(connector, 0, sizeof(mosquitto_connection_t));
    uuid_generate_random(connector->uuid);
    connector->host = (char *) host_node->data.scalar.value;
    connector->port = !port_node? 1883: atoi((char *)port_node->data.scalar.value);
    connector->keepalive = 60;
    connector->clean_session = 1;
    connector->connector_config = &mosquitto_connector_config;
    connector->mosq = mosquitto_new(NULL, connector->clean_session, connector);
    if(!connector->mosq){
        fprintf(stderr, "Error: Out of memory.\n");
        free(connector);
        return NULL;
    } 
    
    mosquitto_connect_callback_set(connector->mosq, __on_connect);
    mosquitto_connect(connector->mosq, connector->host, connector->port, connector->keepalive);
    mosquitto_loop_start(connector->mosq);

    chain_append_key_value(mosquitto_connector_chain, (char *)connector->uuid, connector);

    return connector;
}



static int publish_message(struct mosquitto *mosq, int *mid, const char *topic, int payloadlen, const void *payload, int qos, bool retain){
    return mosquitto_publish(mosq, mid, topic, payloadlen, payload, qos, retain);
}


void * init(yaml_parser_wrapper_t* yaml_parser_wrapper, yaml_node_t *config_node, yaml_node_t *global_config, chain_t *loaded_plugins){
    mosquitto_lib_init();
    if (mosquitto_connector_chain==NULL) mosquitto_connector_chain = new_chain();
    mosquitto_connector_config.yaml_parser_wrapper = yaml_parser_wrapper;
    mosquitto_connector_config.config_node = config_node;
    mosquitto_connector_config.global_config = global_config;
    mosquitto_connector_config.loaded_plugins = loaded_plugins;
    mosquitto_connector_config.new_mosquitto_connection = new_mosquitto_connection;
    mosquitto_connector_config.close_mosquitto_connection = close_mosquitto_connection;
    mosquitto_connector_config.subscribe             = mosquitto_subscribe;
    mosquitto_connector_config.set_message_callback  = mosquitto_message_callback_set;
    mosquitto_connector_config.set_subscribe_callback  = mosquitto_subscribe_callback_set;
    mosquitto_connector_config.set_user_data         = mosquitto_user_data_set;
    mosquitto_connector_config.publish_message       = publish_message;

    return &mosquitto_connector_config;
}
