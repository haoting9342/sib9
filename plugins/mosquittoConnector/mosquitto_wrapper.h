#include <mosquitto.h>

typedef struct mosquitto_connection_s mosquitto_connection_t;

typedef struct mosquitto_connector_config_s {
    yaml_parser_wrapper_t* yaml_parser_wrapper;
    yaml_node_t *config_node;
    yaml_node_t *global_config;
    chain_t *loaded_plugins;
    mosquitto_connection_t * (*new_mosquitto_connection)(yaml_node_t *mosquitto_config_node);
    int (*close_mosquitto_connection)(mosquitto_connection_t *connector);
    void (*set_message_callback)(struct mosquitto *mosq, void (*on_message)(struct mosquitto *, void *, const struct mosquitto_message *));
    void (*set_subscribe_callback)(struct mosquitto *mosq, void (*on_subscribe)(struct mosquitto *, void *, int, int, const int *));
    void (*set_user_data)(struct mosquitto *mosq, void *userdata);
    int (*subscribe)(struct mosquitto *mosq, int *mid, const char *sub, int qos);
    int (*publish_message)(struct mosquitto *mosq, int *mid, const char *, int , const void *, int , bool );
} mosquitto_connector_config_t;


typedef struct mosquitto_connection_s {
    unsigned char uuid[16];
    char *host;
    int port;
    int keepalive;
    int clean_session;
    int retain;
    struct mosquitto *mosq;
    int connected;
    const mosquitto_connector_config_t *connector_config;
}mosquitto_connection_t;
