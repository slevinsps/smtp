#ifndef SMTP_MTA_MAILDIR_H
#define SMTP_MTA_MAILDIR_H

#include "mail.h"

void save_mail_to_maildir( mail* mail );

char* make_subdir_if_not_exists( char* path_to_root_dir, char* new_dir_name );
char* make_maildir_for_user( char* path_to_root_dir, char* user_address );
char* get_path_to_mail_file( char* path_to_user_maildir, char* dirname, char* filename );
char* generate_mail_filename();

#endif //SMTP_MTA_MAILDIR_H
