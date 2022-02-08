#ifndef SMTP_MTA_CLIENT_INFO_H
#define SMTP_MTA_CLIENT_INFO_H

#include "autogen/server-fsm.h"
#include "mail.h"

typedef struct client_description client_description;
struct client_description {
    te_smtp_server_state smtp_state;
    int socket_fd;
    int buffer_input_len;
    int time;
    char* buffer_input;
    char* buffer_output;
    int sent_output_flag;
    mail* mail;
};
 
void reset_client_info( int client_fd );
void free_client_info( int client_fd );
int add_data_to_buffer( int client_fd, char* data );

#endif //SMTP_MTA_CLIENT_INFO_H
