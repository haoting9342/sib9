#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>   
#include <unistd.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h> 
#include <pthread.h>
//#include "ch36x_lib.h"
#include "pcielib.h"
#include <sys/time.h>
#include <dirent.h>
#include "shell/shell.h"
#include <mosquitto.h>
#include "tsn_mqtt_time_domain/clockadj.h"
#include "tsn_mqtt_time_domain/missing.h"


pthread_t jitter_thread;
int jitter_thread_is_running=0;
static void print_menu(FILE *fp);
void *start_report_jitter(void *arg);

typedef struct jitter_config_s{
	int ptp_device_fd;
	clockid_t ptp_clockId;
	int reporting_interval;
	char * reporting_topic;
	char * host;
	int port;
	struct mosquitto *mosq;
	long time_offset_correction;
}jitter_config_t;

typedef struct irgb_reader_s
{
	yaml_parser_wrapper_t* yaml_parser_wrapper;
	yaml_node_t *config_node;
	yaml_node_t *global_config;
	int DEV_IDX;
	int is_openned;
	jitter_config_t jitter_config;
} irgb_reader_t;

static irgb_reader_t *irgb_reader=NULL;

void InterruptEvent(int DEV_IDX)
{
	//Uptime PC system time
	Update_SystemTime(DEV_IDX);
}


void* init(yaml_parser_wrapper_t* yaml_parser_wrapper, yaml_node_t *config_node, yaml_node_t *global_config, chain_t *loaded_plugins){
		(void)loaded_plugins;
		int DEV_IDX=0;
	    yaml_node_t *extra_node = get_yaml_node_by_key(yaml_parser_wrapper, config_node, "extra");
	    if (!extra_node)
	    {
	        fprintf(stderr, "Failed to get extra node\n");
	        return NULL;
	    }
		yaml_node_t * irgb_device_node = get_yaml_node_by_key(yaml_parser_wrapper, extra_node, "irgb_device");
	    if (irgb_device_node)
	    {
	        if (irgb_device_node->type == YAML_SCALAR_NODE)
	        {
	            DEV_IDX = atoi((char *)irgb_device_node->data.scalar.value);
	        }
	    }

		int ret = Open_Device(DEV_IDX);
		if (ret != 0)
		{
			printf("Open_Device err!\n");
			//return NULL;
		}

		irgb_reader = malloc(sizeof(irgb_reader_t));
		irgb_reader->yaml_parser_wrapper = yaml_parser_wrapper;
		irgb_reader->config_node = config_node;
		irgb_reader->global_config = global_config;
		irgb_reader->DEV_IDX = DEV_IDX;
		irgb_reader->is_openned = ret!=0? 0:1 ;

		yaml_node_t * jitter_node = get_yaml_node_by_key(yaml_parser_wrapper, extra_node, "jitter");
	    if (jitter_node)
	    {
			yaml_node_t * ptp_device_node = get_yaml_node_by_key(yaml_parser_wrapper, jitter_node, "ptp_device");
			yaml_node_t * interval_node = get_yaml_node_by_key(yaml_parser_wrapper, jitter_node, "interval");
			yaml_node_t * reporting_topic_node = get_yaml_node_by_key(yaml_parser_wrapper, jitter_node, "reporting_topic");
			yaml_node_t * host_node = get_yaml_node_by_key(yaml_parser_wrapper, jitter_node, "host");
			yaml_node_t * port_node = get_yaml_node_by_key(yaml_parser_wrapper, jitter_node, "port");			
			yaml_node_t * time_offset_correction_node = get_yaml_node_by_key(yaml_parser_wrapper, jitter_node, "time_offset_correction");	

			if (ptp_device_node&&reporting_topic_node&&interval_node&&host_node){
				if (ptp_device_node->type == YAML_SCALAR_NODE && 
					interval_node->type == YAML_SCALAR_NODE &&  
					reporting_topic_node->type == YAML_SCALAR_NODE && 
					host_node->type == YAML_SCALAR_NODE
					){
					irgb_reader->jitter_config.reporting_interval = atoi((char *)interval_node->data.scalar.value);
					irgb_reader->jitter_config.reporting_topic    = (char *) reporting_topic_node->data.scalar.value;
					irgb_reader->jitter_config.time_offset_correction = time_offset_correction_node!=NULL? atol((char *)time_offset_correction_node->data.scalar.value) : -1;
					char * ptp_device  = (char *)ptp_device_node->data.scalar.value;
					irgb_reader->jitter_config.host = (char *)host_node->data.scalar.value;
					irgb_reader->jitter_config.ptp_device_fd = open(ptp_device, O_RDWR);
					if (irgb_reader->jitter_config.ptp_device_fd < 0) {
						fprintf(stderr, "open ptp device: %s failed\n", ptp_device);
						return irgb_reader;
   					 }	
					 irgb_reader->jitter_config.ptp_clockId = FD_TO_CLOCKID(irgb_reader->jitter_config.ptp_device_fd);
					 irgb_reader->jitter_config.port = port_node && port_node->type == YAML_SCALAR_NODE? atoi((char *)port_node->data.scalar.value):  1883;
					 mosquitto_lib_init();
					 irgb_reader->jitter_config.mosq = mosquitto_new ("jitter", NULL, irgb_reader);
					 if(!irgb_reader->jitter_config.mosq){
						fprintf(stderr, "Error: Out of memory.\n");
						close(irgb_reader->jitter_config.ptp_device_fd);
						irgb_reader->jitter_config.ptp_device_fd = -1;
        				mosquitto_lib_cleanup();
						return irgb_reader;
					} 
					int connect_rc = mosquitto_connect(irgb_reader->jitter_config.mosq, irgb_reader->jitter_config.host, irgb_reader->jitter_config.port, 120);
					if (connect_rc!=MOSQ_ERR_SUCCESS){
						fprintf(stderr, "Unable to connect, erro code is %d.\n", connect_rc);
						mosquitto_destroy(irgb_reader->jitter_config.mosq);
        				mosquitto_lib_cleanup();
						irgb_reader->jitter_config.mosq = NULL;
						close(irgb_reader->jitter_config.ptp_device_fd);
						irgb_reader->jitter_config.ptp_device_fd = -1;
						return irgb_reader;
					}
					char time_char[]= "test message\n";
					mosquitto_publish(irgb_reader->jitter_config.mosq, NULL, "jitter", sizeof(time_char),&time_char,0,false);

				}
			}else{
				irgb_reader->jitter_config.ptp_device_fd = -1;
			}
	    }

		return irgb_reader;
}

void close_so(void *irgb){
	(void) irgb;
	int ret = Close_Device(irgb_reader->DEV_IDX);
	if (ret != 0) {
		printf("Close_Device error.\n");	
	}
	free(irgb_reader);
	mosquitto_destroy(irgb_reader->jitter_config.mosq);
    mosquitto_lib_cleanup();
}

static void print_menu(FILE *fp)
{
	fprintf(fp,"pcie test definition:\n");
    fprintf(fp,"Select from following commands :\n");
	fprintf(fp,"open_interrupt                   -- open pcie interrupt\n");
    fprintf(fp,"close_interrupt                  -- close pcie interrupt\n");
    fprintf(fp,"status                           -- read pcie status\n"); 
	fprintf(fp,"get_time                         -- read pcie time(date & time)\n"); 
	fprintf(fp,"get_timestamp                    -- read pcie time(sec & ns)\n"); 
	fprintf(fp,"set_device <id>                  -- set device id\n"); 
	fprintf(fp,"get_device                       -- get current configured device id\n"); 
	fprintf(fp,"report_jitter                    -- report_jitter to mosquitto server\n"); 
	fprintf(fp,"stop_report_jitter               -- stop reporting jitter to mosquitto server\n"); 
    fprintf(fp,"help                             -- print this list\n");
}


void execute_command(shell_t *shell, char **command_args, FILE *fp){
	(void)shell;
	if (irgb_reader==NULL) return;
	if (strcmp(*command_args, "help") == 0){
		print_menu(fp);
		return;
	}else if (strcmp(*command_args, "set_device") == 0){
		command_args++;
		if (*command_args==NULL){
			fprintf(fp, "set_device error.\n");
			return;
		}
		int DEV_IDX = atoi(*command_args);
		if (DEV_IDX<0 || DEV_IDX>3){
			fprintf(fp, "set_device error.\n");
			return;
		}
		if (irgb_reader->DEV_IDX!=DEV_IDX){
			irgb_reader->DEV_IDX = DEV_IDX;
			if (irgb_reader->is_openned){
				Close_Device(irgb_reader->DEV_IDX);
				irgb_reader->is_openned = 0;
			}
		}
		return;
	}else if (strcmp(*command_args, "get_device") == 0){
		fprintf(fp, "configured Device ID is %d .\n",irgb_reader->DEV_IDX);
		return;
	}

	if (!irgb_reader->is_openned){
		if (Open_Device(irgb_reader->DEV_IDX)==0){
			irgb_reader->is_openned =1;
		}else{
			fprintf(fp, "Open_Device %d error.\n", irgb_reader->DEV_IDX);
			return;
		}
	}
	if (strcmp(*command_args, "open_interrupt") == 0){
		//中断开关打开后，只有系统状态sysStatus为4（锁定）时，板卡才会产生中断
		SetIntRoutine(irgb_reader->DEV_IDX,(void*)InterruptEvent);
		return;
	}else if (strcmp(*command_args, "status") == 0){		
		int sysStatus;
		int SyncAccuacy;
		if(Get_Sys_Status(irgb_reader->DEV_IDX,&sysStatus,&SyncAccuacy)< 0)
			fprintf(fp, "Get_Sys_Status error.\n");
		else
			fprintf(fp, "Get_Sys_Status: sysStatus:%d, SyncAccuacy=%d ns.\n",sysStatus,SyncAccuacy);
		fprintf(fp, "configured Device ID is %d .\n",irgb_reader->DEV_IDX);
	}else if (strcmp(*command_args, "close_interrupt") == 0){
		SetIntRoutine(irgb_reader->DEV_IDX,NULL);
		return;
	}else if (strcmp(*command_args, "get_time") == 0){
		Time_Msg msg;
		if(Get_Sys_Time(irgb_reader->DEV_IDX,&msg) < 0)
			fprintf(fp, "Get_Sys_Time error.\n");
		else
			fprintf(fp, "Get_Sys_Time: %04d-%02d-%02d %02d:%02d:%02d .\n",msg.Year,msg.Month,msg.Day,msg.Hour,msg.Min,msg.Sec);
	}else if (strcmp(*command_args, "get_timestamp") == 0){
		unsigned long sec_ns;
		unsigned long sec_t;
		if(Get_Sys_Time_Sec_And_Ns(irgb_reader->DEV_IDX,&sec_t,&sec_ns) < 0){
			fprintf(fp, "Get_Sys_Time_Sec_And_Ns error.\n");
			return;
		}
		fprintf(fp, "Get_Sys_TimeStamp: %ld.%ld \n",sec_t,sec_ns);
	}else if (strcmp(*command_args, "report_jitter") == 0){
		if (jitter_thread_is_running==0) {
			jitter_thread_is_running=1;
			if (pthread_create(&jitter_thread, NULL, start_report_jitter, fp) != 0)
			{
				fprintf(fp, "Failed to start jitter report\n");
				return ;
			}
		}else{
			fprintf(fp, "reporting to mosquitto server is already running\n");
		}

	}else if (strcmp(*command_args, "stop_report_jitter") == 0){
		if (!jitter_thread_is_running) {
			fprintf(fp, "reporting to mosquitto server is not running\n");
			return;
		}
		jitter_thread_is_running= 0;

	}
	return;
}

void *start_report_jitter(void *arg){
	FILE *fp = (FILE *) arg;
	struct timespec ts_5g, ts_ptp, last_ptp, last_5g;
	long long jitter;

	memset(&ts_5g, 0, sizeof(ts_5g));
	memset(&ts_ptp, 0, sizeof(ts_ptp));
	memset(&last_ptp, 0, sizeof(last_ptp));
	memset(&last_5g, 0, sizeof(last_5g));
	
//	char time_char[20];
	long time_offset_correction = irgb_reader->jitter_config.time_offset_correction;
	while (jitter_thread_is_running && 
			irgb_reader->jitter_config.reporting_interval>0 &&
			irgb_reader->jitter_config.mosq!=NULL &&
			irgb_reader->jitter_config.ptp_device_fd>0 &&
			strlen(irgb_reader->jitter_config.reporting_topic)>0 ){

		//memset(time_char,0, sizeof(time_char));
		char time_char[20]={0};
		if(Get_Sys_Time_Sec_And_Ns(irgb_reader->DEV_IDX,(long unsigned int *)&ts_5g.tv_sec,(long unsigned int *)&ts_5g.tv_nsec) < 0){
			fprintf(fp, "Get_Sys_Time_Sec_And_Ns error.\n");
			jitter_thread_is_running = 0 ;
			return NULL;
		}
		clock_gettime(irgb_reader->jitter_config.ptp_clockId, &ts_ptp);
		 
		if (time_offset_correction>=0){
			jitter = time_offset_correction + (ts_ptp.tv_sec - ts_5g.tv_sec) * 1000000000 + ts_ptp.tv_nsec - ts_5g.tv_nsec ;
		}else{
			jitter =  0;
			time_offset_correction =  - (ts_ptp.tv_sec - ts_5g.tv_sec) * 1000000000 - (ts_ptp.tv_nsec - ts_5g.tv_nsec);
		}
		sprintf(time_char, "%lld", jitter);
		fprintf(fp, "jitter is %s; publish to mosquitto server result is %d ;\n", time_char, 
									mosquitto_publish(irgb_reader->jitter_config.mosq, NULL, "jitter", sizeof(time_char),&time_char,0,false));
		fflush(fp);

		if (last_ptp.tv_sec == 0 && last_ptp.tv_nsec == 0){
			last_ptp = ts_ptp;
			last_5g = ts_5g;
		}else{
			double ratio = ((ts_ptp.tv_sec - last_ptp.tv_sec) * 1000000000.0 + ts_ptp.tv_nsec - last_ptp.tv_nsec)/((ts_5g.tv_sec - last_5g.tv_sec) * 1000000000.0 + ts_5g.tv_nsec - last_5g.tv_nsec);
			fprintf(fp, "ratio is %f; \n\n", ratio);
		}
		
		sleep(irgb_reader->jitter_config.reporting_interval);
	}
	jitter_thread_is_running = 0 ;
	return NULL;
}