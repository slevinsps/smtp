#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "server.h"
#include "network.h"
#include "re_parser.h"
#include "config.h"
#include "error_fail.h"
#include "state_handlers.h"
#include "help_funcs.h"

#include "server-fsm.h"

extern struct server smtp_server;

int initialize_server( int port, const char* logdir, const char* maildir )
{
    printf( "Initializing server..." );
    smtp_server.server_socket_fd = create_socket( port );
    smtp_server.break_loop = 0;
    smtp_server.clients = NULL;
    smtp_server.clients_size = 0;
    smtp_server.client_sockets_fds = NULL;
    smtp_server.read_fds  = ( fd_set* ) malloc( sizeof ( fd_set ) );
    smtp_server.write_fds = ( fd_set* ) malloc( sizeof ( fd_set ) );
    smtp_server.exceptions_fds = ( fd_set* ) malloc( sizeof ( fd_set ) );
    smtp_server.logger.dir = logdir;
    smtp_server.maildir = maildir;

    int res = make_dir((char*)smtp_server.maildir);
    if ( res < 0 ) {
        handle_error( "ERROR in make mail dir!" );
    }
    if ( initialize_reg() == 0 ) {
        handle_error( "ERROR in initialize regular expressions for parser!" );
    }
    if ( initialize_logger( &smtp_server.logger ) < 0 ) {
        handle_error( "ERROR in initialize logger!" );
    }
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Server initialized." );
    return 0;   
}

void server_update_fds()
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Updating fd sets for pselect..." );

    /* Reading server's fd set */

    /* Adding server socket */
    FD_ZERO( smtp_server.read_fds );
    FD_SET( smtp_server.server_socket_fd, smtp_server.read_fds );

    smtp_server.max_fd = smtp_server.server_socket_fd;

    /* Adding clients sockets if exist */
    node* current_client = smtp_server.client_sockets_fds;
    while ( current_client != NULL ) {
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Adding client to fd set." );
        FD_SET( current_client->data, smtp_server.read_fds );
        // FD_SET( current_client->data, smtp_server.write_fds );
        if ( current_client->data > smtp_server.max_fd ) {
            smtp_server.max_fd = current_client->data;
        }
        current_client = current_client->next;
    }

    /* Writing server's fd set */
    FD_ZERO( smtp_server.write_fds );


    /* Expections server's fd sets */
    FD_ZERO( smtp_server.exceptions_fds );
    FD_SET( smtp_server.server_socket_fd, smtp_server.exceptions_fds );

    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Fd sets successfully updated." );
}

int run_server() 
{
    /* Main server loop */
    while ( !smtp_server.break_loop ) {
        server_update_fds();

        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Waiting for pselect..." );
        int res_pselect = pselect( smtp_server.max_fd + 1, smtp_server.read_fds,
                smtp_server.write_fds, smtp_server.exceptions_fds, NULL, NULL );
        
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Pselect : %d", res_pselect );

        switch ( res_pselect ) {
        case -1: 
            handle_error( "ERROR pselect returns -1." ); 
            break;
        case 0:
            handle_error( "ERROR pselect returns 0." );
            break;     
        default:
            /* All select fd sets should be checked. */

            /* Checking server socket. */
            if ( FD_ISSET( smtp_server.server_socket_fd, smtp_server.read_fds ) ) {
                handle_new_connect();
            }

            if ( FD_ISSET( smtp_server.server_socket_fd, smtp_server.exceptions_fds ) ) {
                handle_error( "ERROR exception listen socket fd." );
            }

            /* Checking clients sockets. */
            node* current_client = smtp_server.client_sockets_fds;
            while ( current_client != NULL ) {
                node* next_client = current_client->next;
                if ( FD_ISSET( current_client->data, smtp_server.read_fds ) ) {
                    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "handle client read");
                    handle_client_read( current_client->data );
                } else if ( FD_ISSET( current_client->data, smtp_server.write_fds ) ) {
                    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "handle client write");
                    handle_client_write( current_client->data );
                } else if ( FD_ISSET( current_client->data, smtp_server.exceptions_fds ) ) {
                    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Exception in client with fd %i.", current_client->data );
                    close_client_connection( current_client->data );
                }
                current_client = next_client;
            }

            break;
        }

    }

    close_server();
    return 0;
}

void handle_new_connect() 
{           
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Trying to accept new connection..." );
    int client_socket_fd = accept( smtp_server.server_socket_fd, NULL, 0 );
    if ( client_socket_fd < 0 ) {
        handle_error( "ERROR can not accept client!" );
    }
    smtp_server_step( SMTP_SERVER_ST_INIT, SMTP_SERVER_EV_CONN_ACCEPTED,
                      client_socket_fd, NULL);
    smtp_server.client_sockets_fds = linked_list_add_node( smtp_server.client_sockets_fds,
            client_socket_fd );
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Client accepted and client socket added to clients array." );
}

int handle_client_read(int client_fd)
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Reading message from client with fd %d...", client_fd );

    client_info* client = smtp_server.clients[ client_fd ];

    char buffer[BUFFER_SIZE];
    memset( buffer, 0, BUFFER_SIZE );

    ssize_t received = recv( client_fd, buffer, BUFFER_SIZE, 0 );

    if ( received < 0 ) {
        handle_error( "ERROR can not read data from client!" );
    } else if ( received == 0 ) {
        smtp_server_step( client->smtp_state, SMTP_SERVER_EV_CONN_LOST, client_fd, NULL);
    } else {
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Message \"%s\" received from client.",
               buffer );
        memset( client->buffer_input, 0, BUFFER_SIZE );
        memcpy( client->buffer_input, buffer, received );

        // parse for command and send response
        const char* matchdata = NULL;

        re_commands cmnd = match_reg( client->buffer_input, &matchdata);
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Reg find command %d with match data = %s", cmnd, matchdata);

        te_smtp_server_state next_state;

        next_state = smtp_server_step(client->smtp_state,
                                    (te_smtp_server_event) cmnd, client_fd, matchdata);
        
        if (next_state != SMTP_SERVER_ST_CLOSED && next_state != SMTP_SERVER_ST_INVALID) {
            client->smtp_state = next_state;
            log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "New current state for client %d is %d.", client_fd, client->smtp_state );
        }

        free_match_data(matchdata);
    }

    return 0;
}

int handle_client_write( int client_fd )
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Writing message to client with fd %d...", client_fd );
    client_info* client = smtp_server.clients[ client_fd ];
    if ( !client->sent_output_flag ) {
        response_to_client( client_fd );
    }
    return 0;
}

void close_client_connection( int client_fd )
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Trying to close client connection..." );
    close( client_fd );
    smtp_server.client_sockets_fds = linked_list_delete_node( smtp_server.client_sockets_fds, 
        client_fd );
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Client socket is closed." );
}

void close_server() 
{
    close( smtp_server.server_socket_fd );
    finalize_reg();
    logger_destroy( &smtp_server.logger );
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Server is closed." );
}