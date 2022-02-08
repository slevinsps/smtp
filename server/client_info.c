#include <stdlib.h>
#include <string.h>

#include "client_info.h"
#include "server.h"
#include "error_fail.h"

extern struct server smtp_server;

void reset_client_info( int client_fd ) {
    client_description* client = smtp_server.clients[ client_fd ];
    if (client->mail) {
        free_mail( client->mail );
    }
    client->mail = NULL;

    if ( client->buffer_input != NULL ) {
        free(client->buffer_input);
    }
    client->buffer_input = NULL;
    client->buffer_input_len = 0;
}

void free_client_info( int client_fd ) {
    client_description* client = smtp_server.clients[ client_fd ];
    if ( client == NULL ) {
        return;
    }
    if ( client->buffer_input != NULL ) {
        free(client->buffer_input);
        client->buffer_input = NULL;
    }
    if ( client->buffer_output != NULL ) {
        free(client->buffer_output);
        client->buffer_output = NULL;
    }
    if ( client->mail != NULL ) {
        free_mail( client->mail );
        client->mail = NULL;
    }
    free( client );
    smtp_server.clients[ client_fd ] = NULL;
}

int add_data_to_buffer( int client_fd, char* data )
{
    client_description* client = smtp_server.clients[ client_fd ];
    if ( client->buffer_output == NULL ) {
        client->buffer_output = malloc( BUFFER_SIZE );
        memset(client->buffer_output, 0, BUFFER_SIZE);
    }
    if ( strlen( client->buffer_output ) + strlen( data ) >= BUFFER_SIZE ) {
        handle_error( "Output buffer for client is full" );
    }
    sprintf( client->buffer_output + strlen( client->buffer_output ), "%s", data );
    client->sent_output_flag = 0;
    return 0;
}