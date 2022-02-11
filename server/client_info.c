#include <stdlib.h>
#include <string.h>

#include "client_info.h"
#include "server.h"
#include "error_fail.h"

extern struct server smtp_server;

void reset_client_info( client_struct* client ) {
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



client_struct* linked_list_delete_client( client_struct *head, client_struct *delete_node )
{
    client_struct* temp = head;
    client_struct* prev = NULL;
  
    if (temp != NULL && temp == delete_node) { 
        head = temp->next;   // Change head 
        return head; 
    } 
 
    while (temp != NULL && temp != delete_node) { 
        prev = temp; 
        temp = temp->next; 
    } 
  
    if (temp == NULL) { 
        return head;
    } 
  
    if (prev) {
        prev->next = temp->next; 
    }
  
    return head;
}

void free_client_info( client_struct* client ) {
    if ( client == NULL ) {
        return;
    }

    smtp_server.clients_list = linked_list_delete_client(smtp_server.clients_list, client);

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
    client = NULL;
}

int add_data_to_buffer( client_struct* client, char* data )
{
    if ( client->buffer_output == NULL ) {
        client->buffer_output = calloc( BUFFER_SIZE, sizeof(char));
    }

    int num_to_write = BUFFER_SIZE - 1 - strlen( client->buffer_output);
    snprintf(client->buffer_output + strlen( client->buffer_output ), num_to_write, "%s", data);
    client->sent_output_flag = 0;
    return 0;
}


client_struct* add_client( client_struct *head, client_struct *new_node )
{
    if( new_node == NULL ){
        handle_error("Unable to allocate memory for new fd client_struct");
        return NULL;
    }

    new_node->next = NULL;

    // check for first insertion
    if ( head == NULL ) {
        head = new_node;
    } else {
        client_struct *current = head;
        while ( 1 ) {
            if( current->next == NULL ) {
                current->next = new_node;
                break;
            }
            current = current->next;
        };
    }

    return head;
}
