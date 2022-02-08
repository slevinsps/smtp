#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <errno.h>

#include "reg_exprs.h"
#include "responses.h"
#include "state_handlers.h"
#include "server.h"
#include "config.h"
#include "error_fail.h"
#include "help_funcs.h"
#include "network.h"
#include "maildir.h"
#include "logger.h"

extern struct server smtp_server;

int response_to_client(int client_fd )
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Sending message to client with fd %d...", client_fd );
    client_description* client = smtp_server.clients[ client_fd ];

    ssize_t actual_sent = send( client_fd, client->buffer_output, strlen( client->buffer_output ), MSG_NOSIGNAL );
    if ( actual_sent < 0 && ( errno == EPIPE || errno == EAGAIN ) ) {
        client->sent_output_flag = 0;
        log_info( &smtp_server.logger, LOG_MSG_TYPE_ERROR, "Error while sending message EAGAIN or EPIPE, continue.." );
        return 1; 
    } 

    client->sent_output_flag = 1;
    memset( client->buffer_output, 0, BUFFER_SIZE );

    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Message %s sent to client.", client->buffer_output );
    return 0;
}

int HANDLE_CMND_NOOP( int client_fd, te_smtp_server_state nextState ) {
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command NOOP..." );
    add_data_to_buffer( client_fd, RE_RESP_OK );
    response_to_client( client_fd );
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command NOOP finished." );
    return nextState;
}

int HANDLE_ACCEPTED( int client_fd, te_smtp_server_state nextState )
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling accepted." );

    if ( nonblocking_socket( client_fd ) ) {
        log_info( &smtp_server.logger, LOG_MSG_TYPE_ERROR,"ERROR! Fail to set flag 'O_NONBLOCK' for socket.");
    } else {
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO,"Client's socket set as nonblocking.");
    }

    // realloc array of clients
    if ( smtp_server.max_fd >= smtp_server.clients_size ) {
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Reallocing clients array." );
        smtp_server.clients = realloc( smtp_server.clients,
                ( smtp_server.max_fd / CLIENTS_REALLOC_STEP + 1) * CLIENTS_REALLOC_STEP
                * sizeof( client_description* ) );
        smtp_server.clients_size += CLIENTS_REALLOC_STEP;
    }

    // initialize client_description struct for this client_fd
    client_description* client = malloc( sizeof( client_description ) );
    memset( client, 0, sizeof( client_description ) );
    client->buffer_input = NULL;
    client->buffer_input_len = 0;
    client->smtp_state = SMTP_SERVER_ST_READY;
    client->mail = NULL;

    // and add client to clients[]
    smtp_server.clients[ client_fd ] = client;

    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "%s", "New client accepted." );

    add_data_to_buffer( client_fd, RE_RESP_READY );
    response_to_client( client_fd );

    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "New client current smtp state: %d", smtp_server.clients[ client_fd ]->smtp_state );
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling accepted finished." );
    return nextState;
}

int HANDLE_CMND_HELO( int client_fd, const char* matchdata, te_smtp_server_state nextState )
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command HELO..." );

    const char* host = matchdata;

    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Debug: Host: %s", host );

    

    char* host_reverse = reverse_dns_lookup( client_fd );
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Debug: Peer address is: %s", host_reverse );

    if ( strcmp( host, host_reverse ) == 0 ) {
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Client's (%d) address is verified.", client_fd );
    } else {
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Client's (%d) address is not verified!", client_fd );
    }

    if (host_reverse) {
        free(host_reverse);
    }

    add_data_to_buffer( client_fd, RE_RESP_OK );
    response_to_client( client_fd );

    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command HELO finished." );
    return nextState;
}

int HANDLE_CMND_EHLO( int client_fd, const char* matchdata, te_smtp_server_state nextState )
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handle command EHLO." );

    const char* host = matchdata;

    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Debug: Host: %s", host );

    char* host_reverse = reverse_dns_lookup( client_fd );
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Debug: Peer address is: %s", host_reverse );

    if ( strcmp( host, host_reverse ) == 0 ) {
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Client's (%d) address is verified.\r", client_fd );
    } else {
        // it doesn't matter
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Client's (%d) address is not verified!\r", client_fd );
    }

    if (host_reverse) {
        free(host_reverse);
    }

    // TODO: to add supported smtp commands to response?
    add_data_to_buffer( client_fd, RE_RESP_OK );
    response_to_client( client_fd );

    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command EHLO finished." );
    return nextState;
}

int HANDLE_CMND_MAIL( int client_fd, const char* matchdata, te_smtp_server_state nextState )
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command MAIL..." );
    client_description* client = smtp_server.clients[ client_fd ];

    char* email_address = NULL;
    if ( matchdata && strlen(matchdata) > 0 ) {
        email_address = strdup(matchdata);
        
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Email address: %s.", email_address );
    } else {
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Without email address." );
    }

    // adding sender address to client's mail
    client->mail = malloc(sizeof( mail ) );
    memset( client->mail, 0, sizeof( mail ) );
    client->mail->recepients = NULL;
    client->mail->sender = email_address;

    add_data_to_buffer( client_fd, RE_RESP_OK );
    response_to_client( client_fd );

    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command MAIL finished." );
    return nextState;
}

int HANDLE_CMND_RCPT( int client_fd, const char* matchdata, te_smtp_server_state nextState )
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command RCPT..." );
    client_description* client = smtp_server.clients[ client_fd ];

    char* email_address = NULL;
    if ( matchdata && strlen(matchdata) > 0 ) {
        email_address = strdup(matchdata);;
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Email address: %s.", email_address );
    } else {
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Without email address." );
    }

    if ( client->mail->recepients_num + 1 > MAX_RCPT_CLIENTS ) {
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Client's mail already has max number of recepients! Can't add one more.\r" );
        response_to_client( client_fd );
    } else {
        if ( client->mail->recepients == NULL ) {
            // allocation if adding first recepient
            client->mail->recepients = malloc(
                    sizeof(char *) * MAX_RCPT_CLIENTS); // TODO: change allocation (add realloc)
        }
        client->mail->recepients[ client->mail->recepients_num ] = email_address;
        client->mail->recepients_num++;

        add_data_to_buffer( client_fd, RE_RESP_OK );
        response_to_client( client_fd );
    }

    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command RCPT finished." );
    return nextState;
}

int HANDLE_CMND_DATA( int client_fd, te_smtp_server_state nextState )
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command DATA..." );
    client_description* client = smtp_server.clients[ client_fd ];

    // initializing a little buffer for mail data
    client->mail->data = malloc( sizeof( char ) );
    client->mail->data[ 0 ] = '\0';
    client->mail->data_capacity = 0;

    add_data_to_buffer( client_fd, RE_RESP_START_MAIL );
    response_to_client( client_fd );

    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command DATA finished." );
    return nextState;
}

int HANDLE_MAIL_DATA( int client_fd, const char* matchdata,  te_smtp_server_state nextState )
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling mail data..." );
    client_description* client = smtp_server.clients[ client_fd ];

    add_data_to_mail( client->mail, client->buffer_input, strlen( client->buffer_input ) );


    return nextState;
}

int HANDLE_MAIL_END( int client_fd, te_smtp_server_state nextState )
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling end of mail data..." );
    client_description* client = smtp_server.clients[ client_fd ];

    add_data_to_buffer( client_fd, RE_RESP_OK );
    response_to_client( client_fd );
    save_mail_to_dir( client->mail, smtp_server.maildir);

    reset_client_info( client_fd );
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling end of mail data finished." );
    return nextState;
}

int HANDLE_CMND_VRFY( int client_fd, te_smtp_server_state nextState )
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command VRFY..." );

    add_data_to_buffer( client_fd, RE_RESP_ERR_NOT_IMPL_CMND );
    response_to_client( client_fd );

    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command VRFY finished." );
    return nextState;
}

int HANDLE_CMND_RSET( int client_fd, te_smtp_server_state nextState )
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handle command RSET." );
    reset_client_info( client_fd );
    add_data_to_buffer( client_fd, RE_RESP_OK );
    response_to_client( client_fd );
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command RSET finished." );
    return nextState;
}

int HANDLE_CMND_CLOSE( int client_fd, te_smtp_server_state nextState )
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling close..." );
    add_data_to_buffer( client_fd, RE_RESP_CLOSE );
    response_to_client( client_fd );
    free_client_info( client_fd );
    close_client_connection( client_fd );
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling close finished." );
    return nextState;
}

int HANDLE_ERROR( int client_fd, te_smtp_server_state nextState )
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling error..." );
    add_data_to_buffer( client_fd, RE_RESP_ERR_BAD_SEQ );
    response_to_client( client_fd );
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling error finished." );
    return nextState;
}
