#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <errno.h>
#include <time.h>    // time()

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
#include "client_info.h"

extern struct server smtp_server;

int response_to_client(client_struct* client)
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Sending message to client with fd %d...", client->socket_fd );
    // MSG_NOSIGNAL - Требует не посылать сигнал SIGPIPE, если при работе с ориентированным на поток сокетом другая сторона обрывает соединение. Код ошибки EPIPE возвращается в любом случае.
    ssize_t actual_sent = send( client->socket_fd, client->buffer_output, strlen( client->buffer_output ), MSG_NOSIGNAL ); 
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

int HANDLE_CMND_NOOP( client_struct* client, te_smtp_server_state nextState ) {
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command NOOP..." );
    client->time = time(NULL);

    add_data_to_buffer( client, RE_RESP_OK );
    response_to_client( client );
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command NOOP finished." );
    return nextState;
}


// smtp_server.client_sockets_fds = add_client( smtp_server.client_sockets_fds,
//             client_socket_fd );
int HANDLE_ACCEPTED( client_struct* client, te_smtp_server_state nextState )
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling accepted." );

    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "New client accepted." );

    smtp_server.clients_list = add_client(smtp_server.clients_list, client);

    add_data_to_buffer( client, RE_RESP_READY );
    response_to_client( client );

    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling accepted finished." );
    return nextState;
}

int HANDLE_CMND_HELO( client_struct* client, const char* matchdata, te_smtp_server_state nextState )
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command HELO..." );
    client->time = time(NULL);

    const char* host = matchdata;

    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Debug: Host: %s", host );

    char* host_reverse = reverse_dns_lookup( client->socket_fd );
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Debug: Peer address is: %s", host_reverse );

    if ( strcmp( host, host_reverse ) == 0 ) {
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Client's (%d) address is verified.", client );
    } else {
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Client's (%d) address is not verified!", client );
    }

    if (host_reverse) {
        free(host_reverse);
    }

    add_data_to_buffer( client, RE_RESP_OK );
    response_to_client( client );

    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command HELO finished." );
    return nextState;
}

int HANDLE_CMND_EHLO( client_struct* client, const char* matchdata, te_smtp_server_state nextState )
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handle command EHLO." );
    client->time = time(NULL);
    const char* host = matchdata;

    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Debug: Host: %s", host );

    char* host_reverse = reverse_dns_lookup( client->socket_fd );
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Debug: Peer address is: %s", host_reverse );

    if ( strcmp( host, host_reverse ) == 0 ) {
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Client's (%d) address is verified.\r", client );
    } else {
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Client's (%d) address is not verified!\r", client );
    }

    if (host_reverse) {
        free(host_reverse);
    }

    add_data_to_buffer( client, RE_RESP_OK );
    response_to_client( client );

    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command EHLO finished." );
    return nextState;
}

int HANDLE_CMND_MAIL( client_struct* client, const char* matchdata, te_smtp_server_state nextState )
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command MAIL..." );
    client->time = time(NULL);

    char* email_address = NULL;
    if ( matchdata && strlen(matchdata) > 0 ) {
        email_address = strdup(matchdata);
        
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Email address: %s.", email_address );
    } else {
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Without email address." );
    }

    client->mail = malloc(sizeof( mail ) );
    memset( client->mail, 0, sizeof( mail ) );
    client->mail->recepients = NULL;
    client->mail->sender = email_address;

    add_data_to_buffer( client, RE_RESP_OK );
    response_to_client( client );

    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command MAIL finished." );
    return nextState;
}

int HANDLE_CMND_RCPT( client_struct* client, const char* matchdata, te_smtp_server_state nextState )
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command RCPT..." );
    client->time = time(NULL);
    
    char* email_address = NULL;
    if ( matchdata && strlen(matchdata) > 0 ) {
        email_address = strdup(matchdata);;
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Email address: %s.", email_address );
    } else {
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Without email address." );
    }

    if ( client->mail->recepients_num + 1 > MAX_RCPT_CLIENTS ) {
        log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Client's mail already has max number of recepients! Can't add one more.\r" );
        response_to_client( client );
    } else {
        if ( client->mail->recepients == NULL ) {
            client->mail->recepients = malloc(
                    sizeof(char *) * MAX_RCPT_CLIENTS); // TODO: change allocation (add realloc)
        }
        client->mail->recepients[ client->mail->recepients_num ] = email_address;
        client->mail->recepients_num++;

        add_data_to_buffer( client, RE_RESP_OK );
        response_to_client( client );
    }

    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command RCPT finished." );
    return nextState;
}

int HANDLE_CMND_DATA( client_struct* client, te_smtp_server_state nextState )
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command DATA..." );
    client->time = time(NULL);

    // initializing a little buffer for mail data
    client->mail->data = NULL;
    client->mail->data_capacity = 0;

    add_data_to_buffer( client, RE_RESP_START_MAIL );
    response_to_client( client );

    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command DATA finished." );
    return nextState;
}

int HANDLE_MAIL_DATA( client_struct* client, const char* matchdata,  te_smtp_server_state nextState )
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling mail data..." );

    return nextState;
}

int HANDLE_MAIL_END( client_struct* client, te_smtp_server_state nextState )
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling end of mail data..." );
    client->time = time(NULL);

    add_data_to_buffer( client, RE_RESP_OK );
    response_to_client( client );
    save_mail_to_dir( client->mail, smtp_server.maildir);

    reset_client_info( client );
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling end of mail data finished." );
    return nextState;
}

int HANDLE_CMND_VRFY( client_struct* client, te_smtp_server_state nextState )
{
    client->time = time(NULL);
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command VRFY..." );

    add_data_to_buffer( client, RE_RESP_ERR_NOT_IMPL_CMND );
    response_to_client( client );

    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command VRFY finished." );
    return nextState;
}

int HANDLE_CMND_RSET( client_struct* client, te_smtp_server_state nextState )
{
    client->time = time(NULL);
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handle command RSET." );
    reset_client_info( client );
    add_data_to_buffer( client, RE_RESP_OK );
    response_to_client( client );
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling command RSET finished." );
    return nextState;
}


int HANDLE_CMND_CLOSE( client_struct* client, te_smtp_server_state nextState )
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling close..." );
    add_data_to_buffer( client, RE_RESP_CLOSE );
    response_to_client( client );
    close( client->socket_fd );
    free_client_info( client );
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling close finished." );
    return nextState;
}

int HANDLE_ERROR( client_struct* client, te_smtp_server_state nextState )
{
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling error..." );
    add_data_to_buffer( client, RE_RESP_ERR_BAD_SEQ );
    response_to_client( client );
    log_info( &smtp_server.logger, LOG_MSG_TYPE_INFO, "Handling error finished." );
    return nextState;
}
