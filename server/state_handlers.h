#ifndef SMTP_MTA_SERVER_FSM_HANDLERS_H
#define SMTP_MTA_SERVER_FSM_HANDLERS_H

#include "autogen/server-fsm.h"
#include "client_info.h"

int response_to_client(client_struct* client);

/* SMTP FSM HANDLERS: */

int HANDLE_CMND_NOOP( client_struct* client, te_smtp_server_state nextState );
int HANDLE_ACCEPTED( client_struct* client, te_smtp_server_state nextState );
int HANDLE_CMND_HELO( client_struct* client, const char* matchdata, te_smtp_server_state nextState );
int HANDLE_CMND_EHLO( client_struct* client, const char* matchdata, te_smtp_server_state nextState );
int HANDLE_CMND_MAIL( client_struct* client, const char* matchdata, te_smtp_server_state nextState );
int HANDLE_CMND_RCPT( client_struct* client, const char* matchdata, te_smtp_server_state nextState );
int HANDLE_CMND_DATA( client_struct* client, te_smtp_server_state nextState );
int HANDLE_MAIL_DATA( client_struct* client, const char* matchdata, te_smtp_server_state nextState );
int HANDLE_MAIL_END( client_struct* client, te_smtp_server_state nextState );
int HANDLE_CMND_RSET( client_struct* client, te_smtp_server_state nextState );
int HANDLE_CMND_CLOSE( client_struct* client, te_smtp_server_state nextState );
int HANDLE_CMND_VRFY( client_struct* client, te_smtp_server_state nextState );

int HANDLE_ERROR( client_struct* client, te_smtp_server_state nextState );

#endif //SMTP_MTA_SERVER_FSM_HANDLERS_H
