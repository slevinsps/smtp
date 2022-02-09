#ifndef SMTP_MTA_CLIENT_INFO_H
#define SMTP_MTA_CLIENT_INFO_H

#include "autogen/server-fsm.h"
#include "mail.h"

typedef struct client_struct client_struct;
struct client_struct {
    te_smtp_server_state smtp_state;
    int socket_fd;
    int buffer_input_len;
    int time;
    char* buffer_input;
    char* buffer_output;
    int sent_output_flag;
    mail* mail;
    client_struct* next;
};

client_struct* add_client( client_struct *head, client_struct *new_node ); 
void reset_client_info( client_struct* client );
void free_client_info( client_struct* client );
int add_data_to_buffer( client_struct* client, char* data );

#endif //SMTP_MTA_CLIENT_INFO_H
