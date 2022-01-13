#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "helpers.h"
#include "config.h"
#include "error_fail.h"

char* find_domain( char* buffer ) {

    // cut off the end of line in command buffer
    char* eol = strstr( buffer, "\r\n");
    eol[0] = '\0';
    char* command_data = ( char* ) malloc( BUFFER_SIZE );
    strcpy(command_data, &buffer[5]);
    printf( "Debug: Command data copied: %s\n", command_data );

    // skip spaces and < to find start position
    int start = 0;
    for ( start = 0;
    command_data[ start ] == ' ' || command_data[ start ] == '<';
    start++ );

    int len = strlen( command_data ) - start - 1; // -1 is for '>'
    printf( "Debug: Command data real length: %d\n", len );

    char* host = malloc( len + 1 );
    strncpy( host, &command_data[ start ], len );

    return host;
}

char* concat_strings( char* s1, char* s2, char* sep ) {
    int len1 = strlen( s1 );
    int len2 = strlen(sep);
    int len3 = strlen( s2 );
    char* result = malloc( len1 + len2 + len3 + 1 );
    memcpy( result, s1, len1 );
    memcpy( result + len1, sep, len2 );
    memcpy( result + len1 + len2, s2, len3 );
    result[ len1 + len2 + len3] = '\0';
    return result;
}

int make_dir_if_not_exists( char* path_to_dir)
{
    int res = 0;
    struct stat file_stat;
    if ( stat( path_to_dir, &file_stat ) < 0 ) {
        res = mkdir( path_to_dir, 0700 );
        
    }
    return res;
}