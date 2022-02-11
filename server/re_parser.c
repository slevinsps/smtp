#include <stdio.h>
#include <memory.h>

#include "re_parser.h"
#include "error_fail.h"

// Regular Expressions patterns for SMTP commands from re_commands
const char* smtp_re_patterns[ SMTP_RE_CMNDS_COUNTER ] = {
        RE_CMND_NOOP,
        RE_CMND_HELO,
        RE_CMND_EHLO,
        RE_CMND_MAIL,
        RE_CMND_RCPT,
        RE_CMND_VRFY,
        RE_CMND_DATA,
        RE_CMND_RSET,
        RE_CMND_QUIT
    };

// Regular expressions compiled with PCRE2
pcre* smtp_re_compiled[ SMTP_RE_CMNDS_COUNTER ];
pcre_extra* smtp_re_compiled_extra[ SMTP_RE_CMNDS_COUNTER ];

int initialize_reg() {
    printf( "Initializing Reg Expressions with PCRE...\n" );
    int code = 0;

    for ( int re_name = 0; re_name < SMTP_RE_CMNDS_COUNTER; re_name++ ) {
        code |= compile_reg( re_name );
    }
    printf( "Reg Expressions with PCRE Initialized.\n" );
    return !code;
}

int finalize_reg() {
    printf( "Finalizing PCRE regular expressions...\n" );
    for ( int re_name = 0; re_name < SMTP_RE_CMNDS_COUNTER; re_name++ ) {
        free( smtp_re_compiled[ re_name ] );
        free( smtp_re_compiled_extra[ re_name ]);
    }

    printf( "PCRE regular expessions finalized.\n" );
    return 0;
}

int compile_reg( re_commands re_pattern_name )
{
    const char *pcre_error;
    int pcre_error_offset;

    smtp_re_compiled[ re_pattern_name ] = pcre_compile(
        smtp_re_patterns[ re_pattern_name ], PCRE_ANCHORED, &pcre_error, &pcre_error_offset, NULL);

    if ( smtp_re_compiled[ re_pattern_name ] == NULL ) {
        printf( "PCRE compilation failed at offset %d\n", ( int )pcre_error_offset);
        return 1;
    }

    smtp_re_compiled_extra[ re_pattern_name ] = pcre_study(smtp_re_compiled[ re_pattern_name ], 0, &pcre_error);
    if (pcre_error != NULL) {
        printf( "PCRE compilation failed at compiled");
        return -1;
    }

    return 0;
}

re_commands match_reg( const char* text, const char** matchdata, int len_text )
{
    int sub_str_vec[SUB_STR_VEC_LEN];
    int i;
    for (i = 0; i < SMTP_RE_CMNDS_COUNTER; i++) {
        int pcre_ret = pcre_exec(smtp_re_compiled[i], 
            smtp_re_compiled_extra[i], text, len_text, 0, 0, sub_str_vec, SUB_STR_VEC_LEN);

        if (pcre_ret == PCRE_ERROR_NOMEMORY || pcre_ret == PCRE_ERROR_UNKNOWN_NODE) {
            printf( "PCRE compilation failed at compiled\n");
            return (re_commands)SMTP_SERVER_EV_INVALID;
        } else if (pcre_ret > 0) {
            if (pcre_ret > 1) {
                pcre_get_substring(text, sub_str_vec, pcre_ret, 1, matchdata);
            }
            break;
        }
    }

    if (i >= SMTP_RE_CMNDS_COUNTER) {
        i = SMTP_SERVER_EV_INVALID;
    }

    return i;    
}

void free_match_data(const char* match_data) {
    if (match_data) {
        pcre_free_substring(match_data);
    }
}