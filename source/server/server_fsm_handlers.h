#ifndef SMTP_MTA_SERVER_FSM_HANDLERS_H
#define SMTP_MTA_SERVER_FSM_HANDLERS_H

#include "autogen/server-fsm.h"

int send_response_to_client( int client_fd );

/* SMTP FSM HANDLERS: */

int HANDLE_CMND_NOOP( int client_fd, te_smtp_server_state nextState );
int HANDLE_ACCEPTED( int client_fd, te_smtp_server_state nextState );
int HANDLE_CMND_HELO( int client_fd, char*** matchdata, int matchdatalen, te_smtp_server_state nextState );
int HANDLE_CMND_EHLO( int client_fd, char*** matchdata, int matchdatalen, te_smtp_server_state nextState );
int HANDLE_CMND_MAIL( int client_fd, char*** matchdata, int matchdatalen, te_smtp_server_state nextState );
int HANDLE_CMND_RCPT( int client_fd, char*** matchdata, int matchdatalen, te_smtp_server_state nextState );
int HANDLE_CMND_DATA( int client_fd, te_smtp_server_state nextState );
int HANDLE_MAIL_DATA( int client_fd, char*** matchdata, int matchdatalen, int** matchdatasizes, te_smtp_server_state nextState );
int HANDLE_MAIL_END( int client_fd, te_smtp_server_state nextState );
int HANDLE_CMND_RSET( int client_fd, te_smtp_server_state nextState );
int HANDLE_CLOSE( int client_fd, te_smtp_server_state nextState );

int HANDLE_ERROR( int client_fd, te_smtp_server_state nextState );

#endif //SMTP_MTA_SERVER_FSM_HANDLERS_H
