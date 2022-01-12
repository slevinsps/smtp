#include <stdio.h>
#include <memory.h>

#include "re_parser.h"
#include "error_fail.h"

// Regular Expressions patterns for SMTP commands from smtp_re_commands
const char* smtp_re_patterns[ SMTP_RE_CMNDS_COUNTER ] = {
        RE_CMND_NOOP,
        RE_CMND_HELO,
        RE_CMND_EHLO,
        RE_CMND_MAIL,
        RE_CMND_RCPT,
        RE_CMND_VRFY,
        RE_CMND_DATA,
        RE_CMND_RSET,
        RE_CMND_QUIT,
        RE_MAIL_END,
        RE_MAIL_DATA
};

// Regular expressions compiled with PCRE2
pcre2_code* smtp_re_compiled[ SMTP_RE_CMNDS_COUNTER ];

int re_initialize() {
    printf( "Initializing Reg Expressions with PCRE2 library...\n" );
    int code = 0;

    for ( int re_name = 0; re_name < SMTP_RE_CMNDS_COUNTER; re_name++ ) {
        code |= re_compile( re_name );
        printf( "Debug: Re compiled reg exp %d with result code %d.\n", re_name, code );
    }

    return !code;
}

int re_finalize() {
    printf( "Finalizing PCRE2 regular expressions...\n" );
    for ( int re_name = 0; re_name < SMTP_RE_CMNDS_COUNTER; re_name++ ) {
        free( smtp_re_compiled[ re_name ] );
    }

    printf( "PCRE2 regular expessions finalized.\n" );
    return 0;
}

int re_compile( smtp_re_commands re_pattern_name )
{
    int errornumber;
    PCRE2_SIZE erroroffset;

    smtp_re_compiled[ re_pattern_name ] = pcre2_compile(
            ( PCRE2_SPTR )smtp_re_patterns[ re_pattern_name ],   /* the pattern */
            PCRE2_ZERO_TERMINATED, /* indicates pattern is zero-terminated */
            PCRE2_CASELESS,        /* default options */
            &errornumber,          /* for error number */
            &erroroffset,          /* for error offset */
            NULL );                 /* use default compile context */

    if ( !smtp_re_compiled[ re_pattern_name ] ) {
        PCRE2_UCHAR buffer[ 256 ];
        pcre2_get_error_message (errornumber, buffer, sizeof( buffer ) );
        printf( "PCRE2 compilation failed at offset %d: %s\n", ( int )erroroffset,
               buffer );
        return 1;
    }

    return 0;
}

smtp_re_commands re_match_for_command( const char* text, char*** matchdata, int* matchdatalen, int** matchdatasizes )
{
    PCRE2_SPTR subject = ( PCRE2_SPTR )text;
    size_t subject_length = strlen( (  char *) subject );

    int ci;
    int rc;
    pcre2_match_data* match_data;
    PCRE2_SIZE* ovector = 0;
    for ( ci = 0; ci < SMTP_RE_CMNDS_COUNTER; ci++ ) {
        match_data = pcre2_match_data_create_from_pattern( smtp_re_compiled[ ci ], NULL );
        rc = pcre2_match(
                smtp_re_compiled[ ci ],        /* the compiled pattern */
                subject,              /* the subject string */
                subject_length,       /* the length of the subject */
                0,                    /* start at offset 0 in the subject */
                0,                    /* default options */
                match_data,           /* block for storing the result */
                NULL );                /* use default match context */

        if ( rc < 0 && rc != PCRE2_ERROR_NOMATCH ) {
            printf( "matching error %d\n", rc );
            pcre2_match_data_free( match_data );
            fail_on_error( "Re match: error!");
        }

        if ( rc != PCRE2_ERROR_NOMATCH ) {
            ovector = pcre2_get_ovector_pointer( match_data );
            if ( ovector[0] != 0 ) {
                printf( "match found no at offset 0! (at %d, cmd %d)\n", ( int )ovector[ 0 ], ci );
                continue;
            }
            break;
        }
    }

    if ( ci == SMTP_RE_CMNDS_COUNTER ) {
        printf( "no match found!\n" );
        return SMTP_RE_ERROR;
    }
    printf( "Match succeeded at offset %d, command %d\n", ( int )ovector[ 0 ], ci );

    pcre2_code* re = smtp_re_compiled[ ci ];
    if ( rc == 0 ){
        printf( "ovector was not big enough for all the captured substrings\n" );
        fail_on_error( "Re match: error rc == 0." );
    }

    /* See if there are any named substrings, and if so, show them by name. First
    we have to extract the count of named parentheses from the pattern. */

    int namecount;
    int name_entry_size;

    ( void )pcre2_pattern_info(
            re,                   /* the compiled pattern */
            PCRE2_INFO_NAMECOUNT, /* get the number of named substrings */
            &namecount );          /* where to put the answer */

    printf( "Debug: match namecount: %d\n", namecount );

    if ( namecount <= 0 ) {
        return ci;
    }

    *matchdata = calloc( namecount, sizeof( char* ) );
    *matchdatalen = namecount;
    *matchdatasizes = calloc ( namecount, sizeof( int ) );

    PCRE2_SPTR tabptr;

    /* Before we can access the substrings, we must extract the table for
    translating names to numbers, and the size of each entry in the table. */


    printf( "Debug: match proccess substrings..\n" );

    PCRE2_SPTR name_table;
    ( void )pcre2_pattern_info(
            re,                       /* the compiled pattern */
            PCRE2_INFO_NAMETABLE,     /* address of the table */
            &name_table );             /* where to put the answer */

    ( void )pcre2_pattern_info(
            re,                       /* the compiled pattern */
            PCRE2_INFO_NAMEENTRYSIZE, /* size of each entry in the table */
            &name_entry_size );        /* where to put the answer */

    /* Now we can scan the table and, for each entry, print the number, the name,
    and the substring itself. In the 8-bit library the number is held in two
    bytes, most significant first. */

    tabptr = name_table;
    for ( int i = 0; i < namecount; i++ ) {
        int n = ( tabptr[0] << 8 ) | tabptr[ 1 ];

        int sz = ( int )( ovector[ 2 * n + 1 ] - ovector[ 2 * n ] );
        ( *matchdata )[ i ] = calloc( sz + 1, sizeof( char ) );
        ( *matchdatasizes)[ i ] = sz;
        memcpy( ( *matchdata )[ i ], ( char* )( subject + ovector[ 2 * n ] ), sz * sizeof( char ) );

        tabptr += name_entry_size;
    }

    return ci;
}
