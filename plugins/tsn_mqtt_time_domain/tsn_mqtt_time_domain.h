#include "../yaml_wrapper/yaml_wrapper.h"
#include "../../chain.h"
#include "../../plugin_handler.h"
#include "../mosquittoConnector/mosquitto_wrapper.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/ptp_clock.h>


#include <fcntl.h>
#include <unistd.h>

#define MAX_MESSSAGES_NUMBER 100

typedef struct tsn_time_domain_mqtt_msg_s {
    struct timespec mobile_time;
    struct timespec tsn_time;
}tsn_time_domain_mqtt_msg_t;

typedef struct dstt_time_domain_info_s {
    tsn_time_domain_mqtt_msg_t mqtt_time_ref;
    tsn_time_domain_mqtt_msg_t mqtt_time;
    double tsn_time_frequency_delta;
} dstt_time_domain_info_t;

typedef struct tsn_mqtt_time_domain_config_s {
    yaml_parser_wrapper_t* yaml_parser_wrapper;
    yaml_node_t *config_node;
    yaml_node_t *global_config;
    chain_t *loaded_plugins;
    int irgb_device_id;
    clockid_t ptp_tsn_clickid;
    mosquitto_connection_t * mosq_connection;

    dstt_time_domain_info_t messages[MAX_MESSSAGES_NUMBER];
    long current_msg_idx;
    long breaking_time_threshold;
    double sync_interval;
    double beta;
    double alpha;
} tsn_mqtt_time_domain_config_t;

void* init(yaml_parser_wrapper_t* yaml_parser_wrapper, yaml_node_t *config_node, yaml_node_t *global_config, chain_t *loaded_plugins);
void  close_so(void *);

