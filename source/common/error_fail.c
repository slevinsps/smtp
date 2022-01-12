#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "error_fail.h"

void fail_on_error( const char* err )
{
    printf( "Error! %s\n", err );

    switch ( errno ) {
        case EPERM:
            printf( "Operation not permitted.\n" );
            break;
        case EACCES:
            printf( "Permission denied.\n" );
            break;
        default:
            break;
    }

    exit( -1 );
}