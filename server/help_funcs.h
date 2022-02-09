#ifndef SMTP_MTA_HELPERS_H
#define SMTP_MTA_HELPERS_H

char* replace_substring(char* string, int len_string, char* substring, char* replacement);
char* concat_strings( char* s1, char* s2, int len1, int len2, char* sep );
int make_dir( char* path_to_dir);

#endif //SMTP_MTA_HELPERS_H
