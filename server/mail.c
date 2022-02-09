#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "mail.h"
#include "config.h"
#include "help_funcs.h"

void free_mail( mail* mail ) {
    if ( mail->sender != NULL ) {
        free(mail->sender);
        mail->sender = NULL;
    }
    for ( int i = 0; i < mail->recepients_num; i++ ) {
        free( mail->recepients[ i ] );
    }
    if ( mail->recepients != NULL) {
        free( mail->recepients );
        mail->recepients = NULL;
    }
    if ( mail->data != NULL ) {
        free(mail->data);
        mail->data = NULL;
    }
    free( mail );
}

void add_data_to_mail( mail* mail, char* new_data, int new_data_len ) {
    char* new_mail_data = concat_strings("", new_data, 0, new_data_len, "" );
    mail->data = new_mail_data;
    mail->data_capacity = new_data_len;
}


