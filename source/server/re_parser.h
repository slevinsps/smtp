#ifndef SMTP_MTA_PARSER_H
#define SMTP_MTA_PARSER_H

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre.h>  


#include "server-fsm.h"
#include "rexprs.h"

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
    SMTP_RE_MAIL_END = SMTP_SERVER_EV_MAIL_END,
    SMTP_RE_MAIL_DATA = SMTP_SERVER_EV_MAIL_DATA,
    SMTP_RE_ERROR = SMTP_SERVER_EV_INVALID
} smtp_re_commands;
#define SMTP_RE_CMNDS_COUNTER 11 // smtp_re_commands - 1 (error code)
#define SUB_STR_VEC_LEN 10

int re_initialize();
int re_finalize();
int re_compile( smtp_re_commands re_pattern_name );
smtp_re_commands re_match_for_command( const char* text, const char** matchdata );
void free_match_data(const char* match_data);


#endif //SMTP_MTA_PARSER_H
