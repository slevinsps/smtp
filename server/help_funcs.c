#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "help_funcs.h"
#include "config.h"
#include "error_fail.h"


char* concat_strings( char* s1, char* s2, int len1, int len2, char* sep ) {
   
    int sep_len = 0;
    if (sep != NULL) {
        sep_len = strlen( sep );
    }

    char* result = calloc( len1 + len2 + sep_len + 1, sizeof(char) );

    if (s1 != NULL) {
        memcpy( result, s1, len1 );
    }

    if (sep != NULL) {
        memcpy( result + len1, sep, sep_len );
    }
    
    if (s2 != NULL) {
        memcpy( result + len1 + sep_len, s2, len2);
    }
    return result;
}

int make_dir( char* path_to_dir)
{
    int res = 0;
    struct stat file_stat;
    if ( stat( path_to_dir, &file_stat ) < 0 ) {
        res = mkdir( path_to_dir, 0700 );
        
    }
    return res;
}