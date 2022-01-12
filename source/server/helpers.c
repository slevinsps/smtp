#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"
#include "config.h"

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

char* concat_strings( char* s1, char* s2 ) {
    int len1 = strlen( s1 );
    int len2 = strlen( s2 );
    char* result = malloc( len1 + len2 + 1 );
    memcpy( result, s1, len1 );
    memcpy( result + len1, s2, len2 );
    result[ len1 + len2 ] = '\0';
    return result;
}