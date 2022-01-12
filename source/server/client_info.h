#ifndef SMTP_MTA_CLIENT_INFO_H
#define SMTP_MTA_CLIENT_INFO_H

#include "autogen/server-fsm.h"
#include "mail.h"

typedef struct client_info client_info;
struct client_info {
    te_smtp_server_state smtp_state;
    int socket_fd;
    char* buffer_input;
    char* buffer_output;
    int output_is_sent;
    mail* mail;
};

void reset_client_info( int client_fd );
void free_client_info( int client_fd );
int add_data_to_output_buffer( int client_fd, char* data );

#endif //SMTP_MTA_CLIENT_INFO_H
