#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "serveropts.h"

#include "server.h"

struct server smtp_server;

void exit_handler( int signal ) 
{
    // Find out which signal we're handling
    const char* signal_name;
    switch ( signal ) {
        case SIGHUP:
            signal_name = "SIGHUP";
            break;
        case SIGTERM:
            signal_name = "SIGTERM";
            break;
        case SIGQUIT:
            signal_name = "SIGQUIT";
            break;
        case SIGUSR1:
            signal_name = "SIGUSR1";
            break;
        case SIGINT:
            signal_name = "SIGINT";
            break;
        default:
            fprintf( stderr, "Caught wrong signal: %d\n", signal );
            return;
    }

    printf( "\nCaught signal %s, exiting now..\n", signal_name );

    // Close server socket to release port
    server_close();

    exit(0);
}

int set_handler_to_signals() 
{
    struct sigaction sa;

    sa.sa_handler = &exit_handler;
    sa.sa_flags = SA_SIGINFO;

    if ( sigemptyset( &sa.sa_mask ) != 0 )
        return 1;
    if ( sigaddset( &sa.sa_mask, SIGTERM ) != 0 )
        return 1;
    if ( sigaddset( &sa.sa_mask, SIGHUP ) != 0 )
        return 1;
    if ( sigaddset( &sa.sa_mask, SIGQUIT ) != 0 )
        return 1;
    if ( sigaddset( &sa.sa_mask, SIGINT ) != 0 )
        return 1;

    sigaction( SIGTERM, &sa, 0 );
    sigaction( SIGHUP, &sa, 0 );
    sigaction( SIGQUIT, &sa, 0 );
    sigaction( SIGINT, &sa, 0 );

    return 0;
}

int main( int argc, char **argv ) 
{
    int optct = optionProcess( &serverOptions, argc, argv );
    printf( "My options process returns %d.\n", optct );

    int server_port = HAVE_OPT(PORT) ? OPT_VALUE_PORT : SERVER_PORT_DEFAULT;
    printf( "server port flag value: %d.\n", server_port );
    
    const char* logdir = HAVE_OPT(LOGDIR) ? OPT_ARG(LOGDIR) : LOGGER_DIR_DEFAULT;
    printf( "log dir value: %s.\n", logdir );

    const char* maildir = HAVE_OPT(MAILDIR) ? OPT_ARG(MAILDIR) : MAIL_DIR_DEFAULT;
    printf( "mail dir value: %s.\n", maildir );

    if ( set_handler_to_signals() ) {
        printf( "Can not set handlers to signals.\n" );
        return 1;
    }
	printf( "Successfully set exit handler to signals.\n" );

    server_initialize( server_port, logdir, maildir );
    server_run();
    server_close();

	return 0;
}