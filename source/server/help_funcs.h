#ifndef SMTP_MTA_HELPERS_H
#define SMTP_MTA_HELPERS_H

char* find_domain( char* buffer );

char* concat_strings( char* s1, char* s2, char* sep );
int make_dir( char* path_to_dir);

#endif //SMTP_MTA_HELPERS_H
