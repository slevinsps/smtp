#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "serveropts.h"
#include "sigaction.h"
#include "server.h"
#include "error_fail.h"

struct server smtp_server;


int main( int argc, char **argv ) 
{
    optionProcess( &serverOptions, argc, argv );

    int server_port = HAVE_OPT(PORT) ? OPT_VALUE_PORT : SERVER_PORT_DEFAULT;
    printf( "server port value: %d.\n", server_port );
    
    const char* logdir = HAVE_OPT(LOGDIR) ? OPT_ARG(LOGDIR) : LOGGER_DIR_DEFAULT;
    printf( "log dir value: %s.\n", logdir );

    const char* maildir = HAVE_OPT(MAILDIR) ? OPT_ARG(MAILDIR) : MAIL_DIR_DEFAULT;
    printf( "mail dir value: %s.\n", maildir );

    logger_t logger;
    logger.dir = logdir;
    
    if ( initialize_logger( &logger ) < 0 ) {
        handle_error( "ERROR in initialize logger!" );
    }

    initialize_server( server_port, maildir, &logger );
    if ( set_signals_handler() ) {
        printf( "Error in set handlers to signals\n" );
        return 1;
    }
	printf( "Set handlers to signals\n" );

    run_server();

	return 0;
}