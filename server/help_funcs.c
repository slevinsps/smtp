#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>

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



char* replace_substring(char* string, int len_string, char* substring, char* replacement) {
    size_t len1 = strlen(substring);
    size_t len2 = strlen(replacement);
    assert(len1 >= len2);
    
    char* res = calloc(len_string, sizeof(char));
  
    char* ptr;
    char* tmp_res = res;
    char* tmp_string = string;
    

    while ((ptr = strstr(string, substring)) != NULL){
        strncpy(res, string, (size_t)(ptr - string));
        res += ptr - string;
        strncpy(res, replacement, len2);
        res += len2;
        string  = ptr + len1;
    }

    strncpy(res, string, (size_t)(len_string - (string - tmp_string)));
    
    return tmp_res;
}