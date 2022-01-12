#include <stdlib.h>
#include <string.h>

#include "client_info.h"
#include "server.h"
#include "error_fail.h"

extern struct server my_server;

void reset_client_info( int client_fd ) {
    client_info* client = my_server.clients[ client_fd ];
    free_mail( client->mail );
    client->mail = NULL;
}

void free_client_info( int client_fd ) {
    client_info* client = my_server.clients[ client_fd ];
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
    my_server.clients[ client_fd ] = NULL;
}

int add_data_to_output_buffer( int client_fd, char* data )
{
    client_info* client = my_server.clients[ client_fd ];
    if ( client->buffer_output == NULL ) {
        client->buffer_output = malloc( BUFFER_SIZE );
        memset(client->buffer_output, 0, BUFFER_SIZE);
    }
    if ( strlen( client->buffer_output ) + strlen( data ) >= BUFFER_SIZE ) {
        fail_on_error( "Output buffer for client is full" );
    }
    sprintf( client->buffer_output + strlen( client->buffer_output ), "%s", data );
    client->output_is_sent = 0;
    return 0;
}