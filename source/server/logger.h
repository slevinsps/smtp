#ifndef SMTP_MTA_LOGGER_H
#define SMTP_MTA_LOGGER_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "config.h"

#define LOG_MSG_EXIT "exit"

typedef enum {
    LOG_MSG_TYPE_UNDEF = 0,
    LOG_MSG_TYPE_DEBUG = 1,
    LOG_MSG_TYPE_INFO = 2,
    LOG_MSG_TYPE_ERROR = 3,
    LOG_MSG_TYPE_DEFAULT = LOG_MSG_TYPE_DEBUG
} log_msg_type_t;

typedef enum {
    LOG_MSG_LVL_DEBUG = 0,
    LOG_MSG_LVL_INFO = 1,
    LOG_MSG_LVL_ERROR = 2
} log_msg_level_t;

typedef struct log_msg_buffer {
    long msg_type;
    char msg_text[ LOGGER_MSG_CAPACITY ];
} log_msg_t;

typedef struct {
    log_msg_level_t log_level;
    /* log file */
    const char *dir;
    char* filename;
    FILE* file;
    /* Sys Msg Queue data for log messages */
    key_t msg_queue_key;
    int msg_queue_id;
} logger_t;

int initialize_logger( logger_t* logger_sender );
int log_info( logger_t* logger, log_msg_type_t msg_type, const char *format, ...);
void logger_destroy( logger_t* logger );

int logger_init_mq( logger_t* logger );
void logger_run_loop( logger_t* logger );
int logger_open_file( logger_t* logger );

char* logger_get_log_time();
char* logger_get_log_type( log_msg_type_t type );

#endif //SMTP_MTA_LOGGER_H
