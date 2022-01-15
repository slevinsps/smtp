#ifndef SMTP_MTA_REXPRS_H
#define SMTP_MTA_REXPRS_H

/* COMMON: */

#define RE_CRLF "\\r\\n"
#define RE_SPACE "\\s*"
#define RE_DOMAIN "(?<domain>.+)"
#define RE_EMAIL_OR_EMPTY "<(?<address>.+@.+)>|<>"
#define RE_EMAIL "<(?<address>.+@.+)>"
#define RE_DATA "[\\x00-\\x7F]+"
#define RE_DATA_AND_END_OR_DATA "((?<data>[\\x00-\\x7F]+)(?<end>\\r\\n\\.\\r\\n))|([\\x00-\\x7F]+\\r\\n)"

/* COMMANDS: */

#define RE_CMND_NOOP "^NOOP" RE_CRLF
#define RE_CMND_HELO "^HELO" RE_SPACE RE_DOMAIN RE_CRLF
#define RE_CMND_EHLO "^EHLO" RE_SPACE RE_DOMAIN RE_CRLF
#define RE_CMND_MAIL "^MAIL FROM:" RE_SPACE RE_EMAIL_OR_EMPTY RE_CRLF
#define RE_CMND_RCPT "^RCPT TO:" RE_SPACE RE_EMAIL RE_CRLF
#define RE_CMND_VRFY "^VRFY:" RE_SPACE RE_DOMAIN RE_CRLF
#define RE_CMND_DATA "^DATA" RE_CRLF
#define RE_CMND_RSET "^RSET" RE_CRLF
#define RE_CMND_QUIT "^QUIT" RE_CRLF

/* MAIL CONTENT: */

#define RE_MAIL_END "^\\." RE_CRLF
#define RE_MAIL_DATA RE_DATA_AND_END_OR_DATA

#endif //SMTP_MTA_REXPRS_H
