#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <errno.h>

#include "rexprs.h"
#include "server_fsm_handlers.h"
#include "server.h"
#include "config.h"
#include "error_fail.h"
#include "helpers.h"
#include "network.h"
#include "maildir.h"
#include "logger.h"

extern struct server smtp_server;

int send_response_to_client(int client_fd )
{
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Trying to send message to client with fd %d...", client_fd );
    client_info* client = smtp_server.clients[ client_fd ];

    ssize_t actual_sent = send( client_fd, client->buffer_output, strlen( client->buffer_output ), 0 );

    if ( actual_sent < 0 && ( errno == EWOULDBLOCK || errno == EAGAIN ) ) {
        client->output_is_sent = 0;
        logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Error while sending message (EWOULDBLOCK or EAGAIN), continue.." );
        return 1; // TODO: add handling this return code!
    } else if ( actual_sent < 0 && ( errno == EWOULDBLOCK || errno == EAGAIN ) ) {
        fail_on_error( "Client socket send() error." );
    }

    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Actual sent size: %zd", actual_sent );
    // TODO: check actual_sent and delete data that was sent from output buffer

    client->output_is_sent = 1;
    memset( client->buffer_output, 0, BUFFER_SIZE );

    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Message %s sent to client.", client->buffer_output );
    return 0;
}

int HANDLE_CMND_NOOP( int client_fd, te_smtp_server_state nextState ) {
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command NOOP..." );
    add_data_to_output_buffer( client_fd, RE_RESP_OK );
    send_response_to_client( client_fd );
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command NOOP finished." );
    return nextState;
}

int HANDLE_ACCEPTED( int client_fd, te_smtp_server_state nextState )
{
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling accepted." );

    if ( set_socket_as_nonblocking( client_fd ) ) {
        logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO,"ERROR! Fail to set flag 'O_NONBLOCK' for socket.");
    } else {
        logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO,"Client's socket set as nonblocking.");
    }

    // realloc array of clients
    if ( smtp_server.max_fd >= smtp_server.clients_size ) {
        logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Reallocing clients array." );
        smtp_server.clients = realloc( smtp_server.clients,
                ( smtp_server.max_fd / CLIENTS_REALLOC_STEP + 1) * CLIENTS_REALLOC_STEP
                * sizeof( client_info* ) );
        smtp_server.clients_size += CLIENTS_REALLOC_STEP;
    }

    // initialize client_info struct for this client_fd
    client_info* client = malloc( sizeof( client_info ) );
    memset( client, 0, sizeof( client_info ) );
    client->buffer_input = malloc( BUFFER_SIZE );
    memset( client->buffer_input, 0, BUFFER_SIZE );
    client->smtp_state = SMTP_SERVER_ST_READY;
    client->mail = NULL;

    // and add client to clients[]
    smtp_server.clients[ client_fd ] = client;

    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "%s", "New client accepted!!!!!" );

    add_data_to_output_buffer( client_fd, RE_RESP_READY );
    send_response_to_client( client_fd );

    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "New client current smtp state: %d", smtp_server.clients[ client_fd ]->smtp_state );
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling accepted finished." );
    return nextState;
}

int HANDLE_CMND_HELO( int client_fd, char*** matchdata, int matchdatalen, te_smtp_server_state nextState )
{
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command HELO..." );

    // TODO: add DNS checking

    /* compare command data address and real client ip address */

    char* host = NULL;
    if ( matchdatalen == 1 ) {
        host = ( *matchdata )[ matchdatalen - 1 ];
    }
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Debug: Host: %s", host );

    char* host_ip = get_socket_ip_address( client_fd );
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Debug: Peer's IP address is: %s", host_ip );

    if ( strcmp( host, host_ip ) == 0 ) {
        logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Client's (%d) address is verified.", client_fd );
    } else {
        // it doesn't matter
        logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Client's (%d) address is not verified!", client_fd );
    }

    add_data_to_output_buffer( client_fd, RE_RESP_OK );
    send_response_to_client( client_fd );

    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command HELO finished." );
    return nextState;
}

int HANDLE_CMND_EHLO( int client_fd, char*** matchdata, int matchdatalen, te_smtp_server_state nextState )
{
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handle command EHLO." );
    // TODO: add DNS checking

    /* compare command data address and real client ip address */

    char* host = NULL;
    if ( matchdatalen == 1 ) {
        host = ( *matchdata )[ matchdatalen - 1 ];
    }
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Debug: Host: %s", host );

    char* host_ip = get_socket_ip_address( client_fd );
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Debug: Peer's IP address is: %s", host_ip );

    if ( strcmp( host, host_ip ) == 0 ) {
        logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Client's (%d) address is verified.\r", client_fd );
    } else {
        // it doesn't matter
        logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Client's (%d) address is not verified!\r", client_fd );
    }

    // TODO: to add supported smtp commands to response?
    add_data_to_output_buffer( client_fd, RE_RESP_OK );
    send_response_to_client( client_fd );

    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command EHLO finished." );
    return nextState;
}

int HANDLE_CMND_MAIL( int client_fd, char*** matchdata, int matchdatalen, te_smtp_server_state nextState )
{
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command MAIL..." );
    client_info* client = smtp_server.clients[ client_fd ];

    char* email_address = NULL;
    if ( matchdatalen == 1 && ( strcmp(( *matchdata )[ matchdatalen - 1 ], "") != 0 ) ) {
        email_address = ( *matchdata )[ matchdatalen - 1 ];
        logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Debug: 'Mail from' email address: %s.", email_address );
    } else {
        logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Debug: 'Mail from' without email address." );
    }

    // adding sender address to client's mail
    client->mail = malloc(sizeof( mail ) );
    memset( client->mail, 0, sizeof( mail ) );
    client->mail->recepients = NULL;
    client->mail->sender = email_address;

    add_data_to_output_buffer( client_fd, RE_RESP_OK );
    send_response_to_client( client_fd );

    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command MAIL finished." );
    return nextState;
}

int HANDLE_CMND_RCPT( int client_fd, char*** matchdata, int matchdatalen, te_smtp_server_state nextState )
{
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command RCPT..." );
    client_info* client = smtp_server.clients[ client_fd ];

    char* email_address = NULL;
    if ( matchdatalen == 1 && ( strcmp(( *matchdata )[ matchdatalen - 1 ], "") != 0 ) ) {
        email_address = ( *matchdata )[ matchdatalen - 1 ];
        logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Debug: 'Rcpt to' email address: %s.", email_address );
    } else {
        logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Debug: 'Rcpt to' without email address." );
    }

    if ( client->mail->recepients_num + 1 > MAX_RCPT_CLIENTS ) {
        logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Client's mail already has max number of recepients! Can't add one more.\r" );
        send_response_to_client( client_fd );
    } else {
        if ( client->mail->recepients == NULL ) {
            // allocation if adding first recepient
            client->mail->recepients = malloc(
                    sizeof(char *) * MAX_RCPT_CLIENTS); // TODO: change allocation (add realloc)
        }
        client->mail->recepients[ client->mail->recepients_num ] = email_address;
        client->mail->recepients_num++;

        add_data_to_output_buffer( client_fd, RE_RESP_OK );
        send_response_to_client( client_fd );
    }

    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command RCPT finished." );
    return nextState;
}

int HANDLE_CMND_DATA( int client_fd, te_smtp_server_state nextState )
{
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command DATA..." );
    client_info* client = smtp_server.clients[ client_fd ];

    // initializing a little buffer for mail data
    client->mail->data = malloc( sizeof( char ) );
    client->mail->data[ 0 ] = '\0';
    client->mail->data_capacity = 0;

    add_data_to_output_buffer( client_fd, RE_RESP_START_MAIL );
    send_response_to_client( client_fd );

    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command DATA finished." );
    return nextState;
}

int HANDLE_MAIL_DATA( int client_fd, char*** matchdata, int matchdatalen, int** matchdatasizes, te_smtp_server_state nextState )
{
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling mail data..." );
    client_info* client = smtp_server.clients[ client_fd ];

    // TODO: check if two dots - delete one? (rfc 821)

    if ( matchdatalen == 2 ) {
        char* mail_data = ( *matchdata )[ matchdatalen - 2 ];

        if ( *( matchdatasizes[ matchdatalen - 2] ) == 0 ) {
            append_data_to_mail( client->mail, client->buffer_input, strlen( client->buffer_input ) );
        } else {
            append_data_to_mail( client->mail, mail_data, *( matchdatasizes[ matchdatalen - 2] ) );
        }

        char* mail_end = ( *matchdata )[ matchdatalen - 1 ];
        if ( strlen( mail_end ) > 0 ) {
            nextState = smtp_server_step( client->smtp_state,
                    SMTP_SERVER_EV_MAIL_END, client_fd, matchdata, matchdatalen, matchdatasizes );
        }
    } else {
        append_data_to_mail( client->mail, client->buffer_input, strlen( client->buffer_input ) );
    }

    return nextState;
}

int HANDLE_MAIL_END( int client_fd, te_smtp_server_state nextState )
{
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling end of mail data..." );
    client_info* client = smtp_server.clients[ client_fd ];

    add_data_to_output_buffer( client_fd, RE_RESP_OK );
    send_response_to_client( client_fd );
    save_mail_to_maildir( client->mail, smtp_server.maildir);

    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling end of mail data finished." );
    return nextState;
}

int HANDLE_CMND_RSET( int client_fd, te_smtp_server_state nextState )
{
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handle command RSET." );
    reset_client_info( client_fd );
    add_data_to_output_buffer( client_fd, RE_RESP_OK );
    send_response_to_client( client_fd );
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command RSET finished." );
    return nextState;
}

int HANDLE_CLOSE( int client_fd, te_smtp_server_state nextState )
{
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling close..." );
    add_data_to_output_buffer( client_fd, RE_RESP_CLOSE );
    send_response_to_client( client_fd );
    free_client_info( client_fd );
    close_client_connection( client_fd );
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling close finished." );
    return nextState;
}

int HANDLE_ERROR( int client_fd, te_smtp_server_state nextState )
{
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling error..." );
    add_data_to_output_buffer( client_fd, RE_RESP_ERR_BAD_SEQ );
    send_response_to_client( client_fd );
    logger_log_msg( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling error finished." );
    return nextState;
}
