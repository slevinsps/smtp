#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stdlib.h>  // rand(), srand()
#include <time.h>    // time()

#include "logger.h"
#include "error_fail.h"
#include "helpers.h"
#include "server.h"

#include "serveropts.h"



int logger_fork_and_initialize( logger_t* logger_sender )
{
    printf( "Logger: fork and init.\n" );
    pid_t pid;
    switch ( pid = fork() ) {
        case -1:
            printf( "ERROR! Logger fork() failed.\n" );
            return 0;
        case 0:  // logger
            printf("logger process forked with pid: %d\n", getpid());
            printf("parent pid: %d\n", getppid());

            logger_t logger_listener;

            logger_init_mq( &logger_listener );
            logger_listener.dir = logger_sender->dir;
            logger_open_file( &logger_listener );
            logger_run_loop( &logger_listener );
            logger_destroy( &logger_listener );

            kill( getpid(), SIGTERM );

        default: // server
            logger_init_mq( logger_sender );
            break;
    }
    return pid;
}

int logger_init_mq( logger_t* logger )
{
    printf( "Logger: initializing...\n" );

    /* create Sys MQ for log messages */
    logger->msg_queue_key = LOGGER_QUEUE_KEY;

    if ( ( logger->msg_queue_id = msgget( logger->msg_queue_key, 0666 | IPC_CREAT) ) < 0) {
        fail_on_error("logger_initialize(): msgget()");
    }

    printf( "Logger: initialized ok.\n" );
    return 0;
}

void logger_destroy( logger_t* logger )
{
    printf( "Finalizing logger...\n" );
    // close log file
    if ( logger->file ) {
        fclose( logger->file );
    }
    // destroy the message queue
    msgctl( logger->msg_queue_id, IPC_RMID, NULL);
    printf( "Logger finalized.\n" );
}

void logger_run_loop( logger_t* logger )
{
    printf( "Running logger loop..\n" );
    /* Sys MQ data for log msg */
    log_msg_t log_msg;
    ssize_t log_msg_sz;
    log_msg_sz = sizeof( log_msg.msg_text );

    /* Logger in loop until exit message will be received */
    while(1) {

        // msgrcv to receive message
        msgrcv( logger->msg_queue_id, &log_msg, log_msg_sz, 1, 0);

        // break loop if received LOG_MSG_EXIT
        if ( strcmp( log_msg.msg_text, LOG_MSG_EXIT ) == 0 ) {
            printf( "Logger: log msg is exit cmnd.\n" );
            break;
        }

        // display and write message to log file
        printf( "%s \n", log_msg.msg_text );
        fprintf( logger->file, "%s\r\n", log_msg.msg_text );
        fflush(logger->file);
    }

    printf( "Running logger loop finished.\n" );
}

char* logger_get_log_time()
{
    char* result = malloc( sizeof(char) * 50 );
    time_t now = time( 0 );
    strftime ( result, 100, "[%Y-%m-%d %H:%M:%S]", localtime ( &now ) );
    return result;
}

char* logger_get_log_type( log_msg_type_t type )
{
    char* result = malloc( sizeof( char ) * 5 );
    switch ( type ) {
        case LOG_MSG_TYPE_DEBUG:
            result = "DEBUG";
            break;
        case LOG_MSG_TYPE_INFO:
            result = "INFO";
            break;
        case LOG_MSG_TYPE_ERROR:
            result = "ERROR";
            break;
        default:
            result = "UNDEF";
            break;
    }
    return result;
}

int logger_open_file( logger_t* logger )
{
    struct stat st = {0};

    if (stat(logger->dir, &st) == -1) {
        mkdir(logger->dir, 0777);
    }

    printf( "Opening log file...\n");
    logger->filename = malloc( sizeof( char ) * 100 );
    sprintf( logger->filename, "%s/SMTP_LOG_%s", logger->dir, logger_get_log_time() );
    printf( "Logger: filename is %s\n", logger->filename );
    FILE* log_fd = fopen( logger->filename, "a" );
    if ( log_fd == NULL ) {
        fail_on_error( "Logger: can't open file!" );
    }
    logger->file = log_fd;
    printf( "Opening log file finished.\n");
    return 0;
}

int logger_log_msg( logger_t* logger, log_msg_type_t msg_type, char* msg)
{
    /* Sys MQ data for log msg */
    log_msg_t log_msg;
    log_msg.msg_type = 1;
    ssize_t log_msg_sz  = sizeof( log_msg.msg_text );

    /* create full log message string */
    char* timestring = logger_get_log_time();
    char* typestring = logger_get_log_type( msg_type );
    sprintf( log_msg.msg_text, "%s\t%s\t%s", timestring, typestring, msg );

    /* Send log message to logger_listener */
    if ( ( msgsnd( logger->msg_queue_id, &log_msg, log_msg_sz, IPC_NOWAIT ) ) < 0 ) {
        printf("ERROR logger_log_msg(): msgsnd()\n");
        return 1;
    }

    return 0;
}
