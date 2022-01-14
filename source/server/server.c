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
#include "server_fsm_handlers.h"
#include "helpers.h"

#include "server-fsm.h"

extern struct server smtp_server;

int server_initialize( int port, const char* logdir, const char* maildir )
{
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Initializing server..." );
    smtp_server.server_socket_fd = create_socket_on_port( port );
    smtp_server.break_main_loop = 0;
    smtp_server.clients = NULL;
    smtp_server.clients_size = 0;
    smtp_server.client_sockets_fds = NULL;
    smtp_server.read_fds_set  = ( fd_set* ) malloc( sizeof ( fd_set ) );
    smtp_server.write_fds_set = ( fd_set* ) malloc( sizeof ( fd_set ) );
    smtp_server.exceptions_fds_set = ( fd_set* ) malloc( sizeof ( fd_set ) );
    smtp_server.logger.dir = logdir;
    smtp_server.maildir = maildir;

    int res = make_dir_if_not_exists((char*)smtp_server.maildir);
    if ( res < 0 ) {
        fail_on_error( "ERROR! can not make mail dir!" );
    }

    if ( re_initialize() == 0 ) {
        fail_on_error( "Can not initialize regular expressions for parser!" );
    }
    if ( logger_fork_and_initialize( &smtp_server.logger ) < 0 ) {
        fail_on_error( "Can not initialize logger!" );
    }
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Server successfully initialized." );
    return 0;   
}

void server_update_fd_sets()
{
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Updating fd sets for select..." );

    /* Reading server's fd set */

    /* Adding server socket */
    FD_ZERO( smtp_server.read_fds_set );
    FD_SET( smtp_server.server_socket_fd, smtp_server.read_fds_set );

    smtp_server.max_fd = smtp_server.server_socket_fd;

    /* Adding clients sockets if exist */
    node* current_client = smtp_server.client_sockets_fds;
    while ( current_client != NULL ) {
        logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Adding client to fd set." );
        FD_SET( current_client->data, smtp_server.read_fds_set );
        // FD_SET( current_client->data, smtp_server.write_fds_set );
        if ( current_client->data > smtp_server.max_fd ) {
            smtp_server.max_fd = current_client->data;
        }
        current_client = current_client->next;
    }

    /* Writing server's fd set */
    FD_ZERO( smtp_server.write_fds_set );


    /* Expections server's fd sets */
    FD_ZERO( smtp_server.exceptions_fds_set );
    FD_SET( smtp_server.server_socket_fd, smtp_server.exceptions_fds_set );

    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Fd sets successfully updated." );
}

int server_run() 
{
    /* Main server loop */
    while ( !smtp_server.break_main_loop ) {
        server_update_fd_sets();

        logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Waiting for pselect activity..." );
        int activity = pselect( smtp_server.max_fd + 1, smtp_server.read_fds_set,
                smtp_server.write_fds_set, smtp_server.exceptions_fds_set, NULL, NULL );
        
        logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Pselect woke up with activity: %d", activity );

        switch ( activity ) {
        case -1: 
            fail_on_error( "Select returns -1." ); // NOTE: remove this for IDE debug
            break;
        case 0:
            fail_on_error( "Select returns 0." );
            break;     
        default:
            /* All select fd sets should be checked. */

            /* Checking server socket. */
            if ( FD_ISSET( smtp_server.server_socket_fd, smtp_server.read_fds_set ) ) {
                handle_new_connection();
            }

            if ( FD_ISSET( smtp_server.server_socket_fd, smtp_server.exceptions_fds_set ) ) {
                fail_on_error( "Exception listen socket fd." );
            }

            /* Checking clients sockets. */
            node* current_client = smtp_server.client_sockets_fds;
            while ( current_client != NULL ) {
                node* next_client = current_client->next;
                if ( FD_ISSET( current_client->data, smtp_server.read_fds_set ) ) {
                    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO,"handle_client_read");
                    handle_client_read( current_client->data );
                } else if ( FD_ISSET( current_client->data, smtp_server.write_fds_set ) ) {
                    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO,"handle_client_write");
                    handle_client_write( current_client->data );
                } else if ( FD_ISSET( current_client->data, smtp_server.exceptions_fds_set ) ) {
                    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Exception in client with fd %i.", current_client->data );
                    close_client_connection( current_client->data );
                }
                current_client = next_client;
            }

            break;
        }

    }

    return 0;
}

void handle_new_connection() 
{           
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Trying to accept new connection..." );
    int client_socket_fd = accept( smtp_server.server_socket_fd, NULL, 0 );
    if ( client_socket_fd < 0 ) {
        fail_on_error( "Can not accept client!" );
    }
    smtp_server_step( SMTP_SERVER_ST_INIT, SMTP_SERVER_EV_CONN_ACCEPTED,
                      client_socket_fd, NULL);
    smtp_server.client_sockets_fds = linked_list_add_node( smtp_server.client_sockets_fds,
            client_socket_fd );
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Client accepted and client socket added to clients array." );
}

int handle_client_read(int client_fd)
{
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Trying to read message from client with fd %d...", client_fd );

    client_info* client = smtp_server.clients[ client_fd ];

    char buffer[BUFFER_SIZE];
    memset( buffer, 0, BUFFER_SIZE );

    ssize_t actual_received = recv( client_fd, buffer, BUFFER_SIZE, 0 );

    if ( actual_received < 0 ) {
        // TODO: Nonblock
        fail_on_error( "Can not read data from client!" );
    } else if ( actual_received == 0 ) {
        smtp_server_step( client->smtp_state, SMTP_SERVER_EV_CONN_LOST, client_fd, NULL);
    } else {
        //logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Message \"%s\" received from client, message lenght: %zd.",
        //        buffer, actual_received );
        memset( client->buffer_input, 0, BUFFER_SIZE );
        memcpy( client->buffer_input, buffer, actual_received );

        // parse for command and send response
        const char* matchdata = NULL;
        smtp_re_commands cmnd = re_match_for_command( client->buffer_input, &matchdata);
        logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Re match for command result cmnd: %d with match data = %s", cmnd, matchdata);

        te_smtp_server_state next_st;
        if ( cmnd == SMTP_RE_MAIL_DATA &&
            client->smtp_state != SMTP_SERVER_ST_WAITING_FOR_DATA ) {
            logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Reg exp returned command is invalid." );
            smtp_server_step( client->smtp_state,
                    SMTP_SERVER_EV_INVALID, client_fd, matchdata);
            next_st = client->smtp_state;
        } else {
            next_st = smtp_server_step(client->smtp_state,
                                       (te_smtp_server_event) cmnd, client_fd, matchdata);
        }
        
        if (next_st != SMTP_SERVER_ST_CLOSED) {
            client->smtp_state = next_st;
            logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "New current state for client %d is %d.", client_fd, client->smtp_state );
        }

        free_match_data(matchdata);
    }

    return 0;
}

int handle_client_write( int client_fd )
{
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Trying to write message to client with fd %d...", client_fd );
    client_info* client = smtp_server.clients[ client_fd ];
    if ( !client->output_is_sent ) {
        send_response_to_client( client_fd );
    }
    return 0;
}

void close_client_connection( int client_fd )
{
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Trying to close client connection..." );
    close( client_fd );
    smtp_server.client_sockets_fds = linked_list_delete_node( smtp_server.client_sockets_fds, 
        client_fd );
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Client socket is closed." );
}

void server_close() 
{
    close( smtp_server.server_socket_fd );
    re_finalize();
    logger_destroy( &smtp_server.logger );
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Server is closed." );
}