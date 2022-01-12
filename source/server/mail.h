#ifndef SMTP_MTA_MAIL_H
#define SMTP_MTA_MAIL_H

typedef struct mail mail;
struct mail {
    char* sender;
    char** recepients;
    int recepients_num;
    char* data;
    int data_capacity;
};

void free_mail( mail* mail );
void append_data_to_mail( mail* mail, char* new_data, int new_data_len );

char* realloc_mail_data_for_length( mail* mail, int len );

#endif //SMTP_MTA_MAIL_H
