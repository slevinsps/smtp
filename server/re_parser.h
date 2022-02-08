#ifndef SMTP_MTA_PARSER_H
#define SMTP_MTA_PARSER_H

#include <pcre.h>  


#include "autogen/server-fsm.h"
#include "reg_exprs.h"

typedef enum {
    SMTP_RE_CMND_NOOP = SMTP_SERVER_EV_CMND_NOOP,
    SMTP_RE_CMND_HELO = SMTP_SERVER_EV_CMND_HELO,
    SMTP_RE_CMND_EHLO = SMTP_SERVER_EV_CMND_EHLO,
    SMTP_RE_CMND_MAIL = SMTP_SERVER_EV_CMND_MAIL,
    SMTP_RE_CMND_RCPT = SMTP_SERVER_EV_CMND_RCPT,
    SMTP_RE_CMND_VRFY = SMTP_SERVER_EV_CMND_VRFY,
    SMTP_RE_CMND_DATA = SMTP_SERVER_EV_CMND_DATA,
    SMTP_RE_CMND_RSET = SMTP_SERVER_EV_CMND_RSET,
    SMTP_RE_CMND_QUIT = SMTP_SERVER_EV_CMND_QUIT,
    SMTP_RE_ERROR = SMTP_SERVER_EV_INVALID
} re_commands;
#define SMTP_RE_CMNDS_COUNTER 9
#define SUB_STR_VEC_LEN 10

int initialize_reg();
int finalize_reg();
int compile_reg( re_commands re_pattern_name );
re_commands match_reg( const char* text, const char** matchdata, int len_texts );
void free_match_data(const char* match_data);


#endif //SMTP_MTA_PARSER_H
