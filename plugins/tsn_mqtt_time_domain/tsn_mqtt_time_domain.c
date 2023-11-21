#include "tsn_mqtt_time_domain.h"
#include "clockadj.h"
#include "missing.h"
#include "shell/shell.h"
#include <math.h>
#include "include/pcielib.h"

static double time_diff(struct timespec t0, struct timespec t1){
    return (t0.tv_sec - t1.tv_sec)*1000000000.0 + t0.tv_nsec - t1.tv_nsec;
}

double get_time_change_ratio( struct timespec  t0,  struct timespec  t1,  struct timespec t2,  struct timespec  t3){
    double ratio = time_diff(t0,t1)/time_diff(t2,t3);
    return ratio;
}

//implement message_callback
static void __on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message){
    (void)mosq;
     
    tsn_mqtt_time_domain_config_t *tsn_mqtt_time_domain_config = (tsn_mqtt_time_domain_config_t *)obj;
    dstt_time_domain_info_t* messages = tsn_mqtt_time_domain_config->messages;
    long current_msg_idx =tsn_mqtt_time_domain_config->current_msg_idx;
    clockid_t ptp_tsn_clickid = tsn_mqtt_time_domain_config->ptp_tsn_clickid;
    if (message->payloadlen){
        memset(&messages[current_msg_idx], 0, sizeof(dstt_time_domain_info_t));
        //read time from local 5g card. 
        Get_Sys_Time_Sec_And_Ns(tsn_mqtt_time_domain_config->irgb_device_id, 
                                (unsigned long *)&messages[current_msg_idx].mqtt_time.mobile_time.tv_sec, 
                                (unsigned long *)&messages[current_msg_idx].mqtt_time.mobile_time.tv_nsec);

        clock_gettime(ptp_tsn_clickid, &messages[current_msg_idx].mqtt_time.tsn_time);

        messages[current_msg_idx].tsn_time_frequency_delta = clockadj_get_freq(ptp_tsn_clickid);
        fprintf(stdout, "\nreceived message with topic %s: \n", message->topic);
        memcpy(&messages[current_msg_idx].mqtt_time_ref, (tsn_time_domain_mqtt_msg_t *)message->payload, sizeof(tsn_time_domain_mqtt_msg_t));

        fprintf(stdout, "ue tsn time is %ld.%ld ; ", messages[current_msg_idx].mqtt_time.tsn_time.tv_sec, messages[current_msg_idx].mqtt_time.tsn_time.tv_nsec);
        fprintf(stdout, "5g time is %ld.%ld ;\n",  messages[current_msg_idx].mqtt_time.mobile_time.tv_sec, messages[current_msg_idx].mqtt_time.mobile_time.tv_nsec);
        
  //      uint64_t jitter = (messages[current_msg_idx].mqtt_time.tsn_time.tv_sec - messages[current_msg_idx].mqtt_time.mobile_time.tv_sec )* 1000000000 + 
  //                      (messages[current_msg_idx].mqtt_time.tsn_time.tv_nsec - messages[current_msg_idx].mqtt_time.mobile_time.tv_nsec); 

        if (current_msg_idx >0){
            long prev_index = current_msg_idx - 1;
            long current_idx = current_msg_idx;
            // // calculate the time speed ratio from mqtt reference time server
            double  ref_timediff_tsn  = time_diff(messages[current_idx].mqtt_time_ref.tsn_time, messages[prev_index].mqtt_time_ref.tsn_time)/1000000000.0;
            double  ref_timediff_5g  = time_diff(messages[current_idx].mqtt_time_ref.mobile_time, messages[prev_index].mqtt_time_ref.mobile_time)/1000000000.0;
            if (ref_timediff_tsn==0 || ref_timediff_5g==0){
                fprintf(stderr, "time difference is zero, skip this message\n");
                return;
            }
            double ref_tsn_freq = tsn_mqtt_time_domain_config->sync_interval / ref_timediff_tsn;
            double ref_5g_freq  = tsn_mqtt_time_domain_config->sync_interval / ref_timediff_5g;

            double expected_ue_tsn_time_diff_2_new_tsn_time = ref_timediff_tsn / ref_timediff_5g * time_diff(messages[current_idx].mqtt_time.mobile_time, messages[prev_index].mqtt_time_ref.mobile_time);
            double r1 = get_time_change_ratio(messages[current_idx].mqtt_time.tsn_time, messages[prev_index].mqtt_time.tsn_time, messages[current_idx].mqtt_time.mobile_time, messages[prev_index].mqtt_time.mobile_time);

            fprintf(stdout, "r1 is %f =  %f/%f;\n", r1, time_diff(messages[current_idx].mqtt_time.tsn_time, messages[prev_index].mqtt_time.tsn_time), 
                                                       time_diff(messages[current_idx].mqtt_time.mobile_time, messages[prev_index].mqtt_time.mobile_time));

            //print expected_ue_tsn_time_diff_2_new_tsn_time 
            fprintf(stdout, "expected_ue_tsn_time_diff_2_new_tsn_time is %f ;\n", expected_ue_tsn_time_diff_2_new_tsn_time);
            if (fabs(expected_ue_tsn_time_diff_2_new_tsn_time/1000000000) > tsn_mqtt_time_domain_config->breaking_time_threshold){
                fprintf(stderr, "time difference is too large, change the time directly\n");
                long long expected_ue_tsn_time = expected_ue_tsn_time_diff_2_new_tsn_time + messages[prev_index].mqtt_time_ref.tsn_time.tv_sec * 1000000000.0 + messages[prev_index].mqtt_time_ref.tsn_time.tv_nsec;
                struct timespec expected_ue_tsn_time_ts = {expected_ue_tsn_time / 1000000000, expected_ue_tsn_time % 1000000000};
                 //set the time of tsn time to the expected value
                clock_settime(ptp_tsn_clickid, &expected_ue_tsn_time_ts);
                messages[current_idx].mqtt_time.tsn_time = expected_ue_tsn_time_ts;
                //update the new time to the messages
                fprintf(stdout, "new ue tsn time is %ld.%ld ;\n", messages[current_idx].mqtt_time.tsn_time.tv_sec, messages[current_idx].mqtt_time.tsn_time.tv_nsec);
                fprintf(stdout, "nw tsn time is %ld.%ld ;\n", messages[current_idx].mqtt_time_ref.tsn_time.tv_sec, messages[current_idx].mqtt_time_ref.tsn_time.tv_nsec);
                fprintf(stdout, "nw 5g time is %ld.%ld ;\n", messages[current_idx].mqtt_time_ref.mobile_time.tv_sec, messages[current_idx].mqtt_time_ref.mobile_time.tv_nsec);
            }

            
            double expected_tsn_time_freq_delta = ref_tsn_freq - r1 * ref_5g_freq + messages[current_idx].tsn_time_frequency_delta;

            //set the frequency of tsn time to the expected value
            clockadj_set_freq(ptp_tsn_clickid, expected_tsn_time_freq_delta);
            messages[current_idx].tsn_time_frequency_delta = expected_tsn_time_freq_delta;

            //print all new tsn time and 5g time and frequency
            fprintf(stdout, "tsn time frequency delta is %f, %f, %f, %f  ;\n", messages[current_idx].tsn_time_frequency_delta, r1, ref_timediff_tsn, ref_timediff_5g);
        }  
        tsn_mqtt_time_domain_config->current_msg_idx++;
        if (tsn_mqtt_time_domain_config->current_msg_idx >= MAX_MESSSAGES_NUMBER){
            tsn_mqtt_time_domain_config->current_msg_idx = 0;
        }
        
    }else{
        fprintf(stderr, "%s (null)\n", message->topic);
    }
    fflush(stdout);
}

//implement subscribe callback
static void __on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos){
    (void)mosq;
    (void)obj;
    (void)mid;
    (void)qos_count;
    (void)granted_qos;
    fprintf(stdout, "Subscription succeeded\n");
    fflush(stdout);
}

void* init(yaml_parser_wrapper_t* yaml_parser_wrapper, yaml_node_t *config_node, yaml_node_t *global_config, chain_t *loaded_plugins){
    mosquitto_connection_t * mosq_connection = NULL;

    int irgb_device_id = 0;
    int tsn_net_ptp_device_fd = 0;
    clockid_t ptp_tsn_clickid;

    yaml_node_t *extra_node = get_yaml_node_by_key(yaml_parser_wrapper, config_node, "extra");
    if (!extra_node){
        fprintf(stderr, "extra is not specified in tsn_mqtt_time_domain config\n");
        return NULL;
    }

    yaml_node_t *ptp_tsn_net_device_node = get_yaml_node_by_key(yaml_parser_wrapper, extra_node, "ptp_tsn_net_device");
    if (!ptp_tsn_net_device_node){
        fprintf(stderr, "ptp_tsn_net_device is not specified in tsn_mqtt_time_domain config\n");
        return NULL;
    }

    //irgb_device
    yaml_node_t *irgb_device_node = get_yaml_node_by_key(yaml_parser_wrapper, extra_node, "irgb_device");
    if (!irgb_device_node){
        fprintf(stderr, "irgb_device is not specified in tsn_mqtt_time_domain config\n");
        return NULL;
    }

    yaml_node_t *mosq_node = get_yaml_node_by_key(yaml_parser_wrapper, extra_node, "mosq");
    if (!mosq_node){
        fprintf(stderr, "mosq is not specified in tsn_mqtt_time_domain config\n");
        return NULL;
    }

    //parse mosq_connector_library node
    yaml_node_t *mosq_connector_library_node = get_yaml_node_by_key(yaml_parser_wrapper, extra_node, "mosq_connector_library");
    if (!mosq_connector_library_node){
        fprintf(stderr, "mosq_connector_library is not specified in tsn_mqtt_time_domain config\n");
        return NULL;
    }

    char *mosq_connector_library = (char *)mosq_connector_library_node->data.scalar.value;
    //find out mosquitto connector node from loaded_plugins by using chain_has_key
    chain_node_t *mosq_connector_plugin_node = chain_has_key(loaded_plugins, mosq_connector_library);
    if (!mosq_connector_plugin_node){
        fprintf(stderr, "mosq_connector_library %s is not loaded\n", mosq_connector_library);
        return NULL;
    }
    
    if (!mosq_connector_plugin_node->value){
        fprintf(stderr, "mosq_connector_library %s is not loaded\n", mosq_connector_library);
        return NULL;
    }
    loaded_plugins_t *mosq_connector_info = (loaded_plugins_t *)mosq_connector_plugin_node->value;

    
    if (!mosq_connector_info->init_result){
        fprintf(stderr, "mosq_connector_library %s is not loaded\n", mosq_connector_library);
        return NULL;
    }
    mosquitto_connector_config_t *mosq_connector_config = (mosquitto_connector_config_t *)mosq_connector_info->init_result;

    mosq_connection= mosq_connector_config->new_mosquitto_connection(mosq_node);

    if (!mosq_connection){
        fprintf(stderr, "mosq_connector_library %s is not loaded\n", mosq_connector_library);
        return NULL;
    }

    // get topic configuration
    yaml_node_t *topic_node = get_yaml_node_by_key(yaml_parser_wrapper, extra_node, "topic");
    char *topic = topic_node==NULL ? "tsn_mqtt_time_domain" : (char *)topic_node->data.scalar.value;
    // print out topic to stand out
    fprintf(stdout, "topic is %s\n", topic);
    
    mosq_connector_config->set_message_callback( mosq_connection->mosq, __on_message);
    mosq_connector_config->set_subscribe_callback (mosq_connection->mosq, __on_subscribe);

    //subscribe topic
    int ret = mosq_connector_config->subscribe(mosq_connection->mosq, NULL, topic, 0);  
    if (ret != MOSQ_ERR_SUCCESS){
        fprintf(stderr, "mosquitto_subscribe failed\n");
        mosq_connector_config->close_mosquitto_connection(mosq_connection);
        return NULL;
    }

    char *ptp_tsn_net_device = (char *)ptp_tsn_net_device_node->data.scalar.value;
    irgb_device_id = atoi((char *)irgb_device_node->data.scalar.value);
        //open ptp device
    tsn_net_ptp_device_fd = open(ptp_tsn_net_device, O_RDWR);
    if (tsn_net_ptp_device_fd < 0) {
        fprintf(stderr, "open tsn ptp device: %s failed\n", ptp_tsn_net_device);
        mosq_connector_config->close_mosquitto_connection(mosq_connection);
        return NULL;
    }
    ptp_tsn_clickid = FD_TO_CLOCKID(tsn_net_ptp_device_fd);

    tsn_mqtt_time_domain_config_t *tsn_mqtt_time_domain_config = malloc(sizeof(tsn_mqtt_time_domain_config_t));
    memset(tsn_mqtt_time_domain_config, 0, sizeof(tsn_mqtt_time_domain_config_t));

    tsn_mqtt_time_domain_config->yaml_parser_wrapper = yaml_parser_wrapper;
    tsn_mqtt_time_domain_config->config_node = config_node;
    tsn_mqtt_time_domain_config->global_config = global_config;
    tsn_mqtt_time_domain_config->loaded_plugins = loaded_plugins;
    tsn_mqtt_time_domain_config->irgb_device_id = irgb_device_id;
    tsn_mqtt_time_domain_config->ptp_tsn_clickid = ptp_tsn_clickid;
    tsn_mqtt_time_domain_config->breaking_time_threshold = 10000000;
    tsn_mqtt_time_domain_config->sync_interval  = 1.0;
    tsn_mqtt_time_domain_config->mosq_connection = mosq_connection;
    
    mosq_connector_config->set_user_data(mosq_connection->mosq, tsn_mqtt_time_domain_config);

    return tsn_mqtt_time_domain_config;

}


void execute_command(shell_t *shell, char **command_args, FILE *fp){
    //char *command_args[MAX_COMMAND_ARGS]
    (void)shell;
    (void)command_args;
    (void)fp;


}