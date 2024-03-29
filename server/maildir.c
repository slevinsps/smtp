#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#include "maildir.h"
#include "help_funcs.h"
#include "config.h"
#include "error_fail.h"
#include "reg_exprs.h"
#include "help_funcs.h"


void save_mail_to_dir( mail* mail, const char* maildir )
{
    printf( "Saving mail to maildir...\n" );
    printf( "Recepients number = %d\n", mail->recepients_num);
    char* maildata = replace_substring(mail->data, mail->data_capacity, EOL_WITH_DOT_MASK, EOL_WITH_DOT);

    for ( int i = 0; i < mail->recepients_num; i++ ) {
        char* path_to_user_maildir = make_maildir_for_user( (char*)maildir, mail->recepients[ i ] );
        char* mail_filename = generate_mail_filename();
        char* path_to_file_in_tmp = get_path_to_mail_file( path_to_user_maildir, "tmp", mail_filename );
        char* path_to_file_in_new = get_path_to_mail_file( path_to_user_maildir, "new", mail_filename );

        FILE* mail_fd = fopen( path_to_file_in_tmp, "a" );
        if (mail->sender) {
            fprintf( mail_fd, "From: <%s>\n", mail->sender );
        } else {
            fprintf( mail_fd, "From: <>\n");
        }
        for ( int j = 0; j < mail->recepients_num; j++ ) {
            fprintf( mail_fd, "To: <%s>\n", mail->recepients[ j ] );
        }
        
        fprintf( mail_fd, "%s", maildata );
        fclose( mail_fd );
        rename( path_to_file_in_tmp, path_to_file_in_new );

        free( path_to_file_in_tmp );
        free( path_to_file_in_new );
        free( mail_filename );
        free( path_to_user_maildir );
    }
    free(maildata);
    printf( "Saving mail to maildir finished.\n" );
}


char* make_subdir_if_not_exists( char* path_to_root_dir, char* new_dir_name )
{
    char* path_to_new_dir = concat_strings( path_to_root_dir, new_dir_name, strlen(path_to_root_dir), strlen(new_dir_name), "/" );
    int res = make_dir(path_to_new_dir);
    if ( res < 0 ) {
        handle_error( "ERROR! can not make dir!\n" );
    }
    return path_to_new_dir;
}

char* make_maildir_for_user( char* path_to_root_dir, char* user_address )
{
    char* path_to_user_dir = make_subdir_if_not_exists( path_to_root_dir, user_address );
    char* path_to_user_maildir = make_subdir_if_not_exists( path_to_user_dir, "Maildir" );
    free( path_to_user_dir );

    char* path_to_subdir = NULL;
    path_to_subdir = make_subdir_if_not_exists( path_to_user_maildir, "tmp" );
    free( path_to_subdir );
    path_to_subdir = make_subdir_if_not_exists( path_to_user_maildir, "new" );
    free( path_to_subdir );

    return path_to_user_maildir;
}

char* get_path_to_mail_file( char* path_to_user_maildir, char* dirname, char* filename )
{
    char* full_path_to_dir = concat_strings( path_to_user_maildir, dirname, strlen(path_to_user_maildir), strlen(dirname), "/" );
    char* path_to_file = concat_strings( full_path_to_dir, filename, strlen(full_path_to_dir), strlen(filename), "/" );
    free( full_path_to_dir );
    return path_to_file;
}

char* generate_mail_filename()
{
    struct timeval tv;
    struct timezone tz;

    gettimeofday(&tv,&tz);
    srand(tv.tv_usec);

    char* filename = malloc( sizeof( char ) * MAX_LEN_FILENAME ); 
    sprintf ( filename, "%lx.%lx.%x", tv.tv_sec, (long) tv.tv_usec, rand() );

    return filename;
}