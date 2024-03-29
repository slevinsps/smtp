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
#include <errno.h>
#include <time.h>    // time()
#include "responses.h"

#include "server.h"
#include "network.h"
#include "re_parser.h"
#include "config.h"
#include "error_fail.h"
#include "state_handlers.h"
#include "help_funcs.h"
#include "network.h"

#include "autogen/server-fsm.h"

extern struct server smtp_server;

int initialize_server( int port, const char* maildir, logger_t *logger_listener )
{
    printf( "Initializing server..." );
    smtp_server.server_fd = create_socket( port );
    smtp_server.clients_list = NULL;
    smtp_server.read_fds  = ( fd_set* ) malloc( sizeof ( fd_set ) );
    smtp_server.write_fds = ( fd_set* ) malloc( sizeof ( fd_set ) );
    smtp_server.exceptions_fds = ( fd_set* ) malloc( sizeof ( fd_set ) );
    smtp_server.maildir = maildir;
    smtp_server.logger = *logger_listener;

    int res = make_dir((char*)smtp_server.maildir);
    if ( res < 0 ) {
        handle_error( "ERROR in make mail dir!" );
    }
    if ( initialize_reg() == 0 ) {
        handle_error( "ERROR in initialize regular expressions for parser!" );
    }
    
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Server initialized." );
    return 0;   
}

void server_update_fds()
{
    FD_ZERO( smtp_server.read_fds );
    FD_ZERO( smtp_server.write_fds );
    FD_SET( smtp_server.server_fd, smtp_server.read_fds );

    smtp_server.max_fd = smtp_server.server_fd;

    client_struct* current_client = smtp_server.clients_list;
    while ( current_client != NULL ) {
        FD_SET( current_client->socket_fd, smtp_server.read_fds );
        FD_SET( current_client->socket_fd, smtp_server.write_fds );
        if ( current_client->socket_fd > smtp_server.max_fd ) {
            smtp_server.max_fd = current_client->socket_fd;
        }
        current_client = current_client->next;
    }

    FD_ZERO( smtp_server.exceptions_fds );
    FD_SET( smtp_server.server_fd, smtp_server.exceptions_fds );
}

int run_server() 
{
    while (1) {
        server_update_fds();

        int res_pselect = pselect( smtp_server.max_fd + 1, smtp_server.read_fds,
                smtp_server.write_fds, smtp_server.exceptions_fds, NULL, NULL );
        
        switch ( res_pselect ) {
        case -1: 
            handle_error( "ERROR pselect returns -1." ); 
            break;    
        default:

            /* Checking server socket. */
            if ( FD_ISSET( smtp_server.server_fd, smtp_server.read_fds ) ) {
                handle_new_connect();
            }

            if ( FD_ISSET( smtp_server.server_fd, smtp_server.exceptions_fds ) ) {
                handle_error( "ERROR exception listen socket fd." );
            }

            /* Checking clients sockets. */
            client_struct* current_client = smtp_server.clients_list;
            while ( current_client != NULL ) {
                client_struct* next_client = current_client->next;

                int time_diff = time(NULL) - current_client->time;
                if (time_diff > TIMEOUT) {
                    smtp_server_step( current_client->smtp_state, SMTP_SERVER_EV_CONN_LOST, current_client, NULL);
                } else {
                    if ( FD_ISSET( current_client->socket_fd, smtp_server.read_fds ) ) {
                        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "handle client read");
                        handle_client_read( current_client );
                    } else if ( FD_ISSET( current_client->socket_fd, smtp_server.write_fds ) ) {
                        handle_client_write( current_client );
                    } else if ( FD_ISSET( current_client->socket_fd, smtp_server.exceptions_fds ) ) {
                        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Exception in client with fd %i.", current_client->socket_fd );
                        smtp_server_step( current_client->smtp_state, SMTP_SERVER_EV_CONN_LOST, current_client, NULL);
                    }
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
    int client_socket_fd = accept( smtp_server.server_fd, NULL, 0 ); // sockaddr - NULL
    if ( client_socket_fd < 0 ) {
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO,"Can not accept client!");
        return;
    }

    if ( nonblocking_socket( client_socket_fd ) ) {
        log_info( &smtp_server.logger, LOG_MSG_TYPE_ERROR,"ERROR! Fail to set flag 'O_NONBLOCK' for socket.");
    } else {
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO,"Client's socket set as nonblocking.");
    }

    client_struct* client = malloc( sizeof( client_struct ) );
    memset( client, 0, sizeof( client_struct ) );
    client->buffer_input = NULL;
    client->buffer_input_len = 0;
    client->buffer_output = NULL;
    client->time = time(NULL);
    client->smtp_state = SMTP_SERVER_ST_READY;
    client->mail = NULL;
    client->next = NULL;
    client->socket_fd = client_socket_fd;
    
    smtp_server_step( SMTP_SERVER_ST_INIT, SMTP_SERVER_EV_CONN_ACCEPTED,
                      client, NULL);
    
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Client accepted and client socket added to clients array." );
}



int handle_command(client_struct* client, te_smtp_server_state* next_state) {

    int message_len = 0;
    char* end = strstr(client->buffer_input, EOL);

    if (end) {
        int end_size = sizeof(EOL) - 1;
        end += end_size;
        message_len = end - client->buffer_input;

        const char* matchdata = NULL;
        re_commands cmnd = match_reg( client->buffer_input, &matchdata, message_len);
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Reg find command %d with match data = %s", cmnd, matchdata);

        *next_state = smtp_server_step(client->smtp_state,
                                    (te_smtp_server_event) cmnd, client, matchdata);
        
        if (*next_state != SMTP_SERVER_ST_CLOSED && *next_state != SMTP_SERVER_ST_INVALID) {
            client->smtp_state = *next_state;
            log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "New current state for client %d is %d.", client->socket_fd, client->smtp_state );
        } else if (*next_state == SMTP_SERVER_ST_INVALID) {
            free(client->buffer_input);
            client->buffer_input = NULL;
            client->buffer_input_len = 0;
        }
        

        free_match_data(matchdata);
    }

    return message_len;
}

int handle_data(client_struct* client) {
    int msg_len = 0;
    char* end = strstr(client->buffer_input, EOM);

    if (end) {
        end[0] = 0;

        int end_size = sizeof(EOM) - 1;
        end += end_size; 
        msg_len = end - client->buffer_input;

        add_data_to_mail( client->mail, client->buffer_input, msg_len );

        te_smtp_server_state next_state = smtp_server_step(client->smtp_state, SMTP_SERVER_EV_MAIL_END, client, NULL);
        
        if (next_state != SMTP_SERVER_ST_CLOSED && next_state != SMTP_SERVER_ST_INVALID) {
            client->smtp_state = next_state;
            log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "New current state for client %d is %d.", client->socket_fd, client->smtp_state );
        }

    }

    return msg_len;
}

int handle_client_read(client_struct* client)
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Reading message from client with fd %d...", client->socket_fd );

    char buffer[BUFFER_SIZE];
    memset( buffer, 0, BUFFER_SIZE );

    ssize_t received = recv( client->socket_fd, buffer, BUFFER_SIZE, 0 );

    if ( received < 0 && errno != EAGAIN) {
        smtp_server_step( client->smtp_state, SMTP_SERVER_EV_CONN_LOST, client, NULL);
    } else if ( received == 0 ) {
        smtp_server_step( client->smtp_state, SMTP_SERVER_EV_CONN_LOST, client, NULL);
    } else {
        // log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Message \"%s\" received from client.",
        //        buffer );

        char* new_buffer = concat_strings(client->buffer_input, buffer, client->buffer_input_len, received, "");
        if (client->buffer_input)
            free(client->buffer_input);

        client->buffer_input = new_buffer;
        client->buffer_input_len += received;
        int message_len = 0;

        
        te_smtp_server_state next_state = SMTP_SERVER_ST_READY;
        if (client->smtp_state == SMTP_SERVER_ST_DATA) {
            message_len = handle_data(client);
        } else {
            message_len = handle_command(client, &next_state);
        }

        if (next_state == SMTP_SERVER_ST_CLOSED) {
            client = NULL;
        }

        if (message_len > 0 && client && client->buffer_input) {
            if (message_len != client->buffer_input_len) {
                client->buffer_input_len -= message_len;
                memcpy(client->buffer_input, client->buffer_input + message_len, sizeof(char) * client->buffer_input_len);
            } else {
                free(client->buffer_input);
                client->buffer_input = NULL;
                client->buffer_input_len = 0;
            }
            
        }

        // printf(" afterclient->buffer_input = %s  %d\n", client->buffer_input, client->buffer_input_len);

    }

    return 0;
}


int handle_client_write(client_struct* client)
{
    if ( !client->sent_output_flag ) {
        response_to_client( client);
    }
    return 0;
}



void close_server() 
{
    close( smtp_server.server_fd );
    finalize_reg();
    logger_destroy( &smtp_server.logger );

    client_struct* client = smtp_server.clients_list;
    client_struct* next_client;
    while ( client != NULL ) {
        close( client->socket_fd );
        next_client = client->next;
        free_client_info(client);
        client = next_client;
    }

    if (smtp_server.read_fds)
        free(smtp_server.read_fds);
    if (smtp_server.write_fds)
        free(smtp_server.write_fds);
    if (smtp_server.exceptions_fds)
        free(smtp_server.exceptions_fds);
    if (smtp_server.clients_list)
        free(smtp_server.clients_list);

}