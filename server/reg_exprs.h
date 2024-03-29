#ifndef SMTP_MTA_REXPRS_H
#define SMTP_MTA_REXPRS_H

/* COMMON: */

#define RE_CRLF "\\r\\n"
#define RE_SPACE "\\s*"
#define RE_DOMAIN "(?<domain>.+)"
#define RE_EMAIL_OR_EMPTY "<(?<address>.+@.+)?>"
#define RE_EMAIL "<(?<address>.+@.+)>"
#define RE_DATA "[\\x00-\\x7F]+"

/* COMMANDS: */

#define RE_CMND_NOOP "[Nn][Oo][Oo][Pp]" RE_CRLF
#define RE_CMND_HELO "[Hh][Ee][Ll][Oo]" RE_SPACE RE_DOMAIN RE_CRLF
#define RE_CMND_EHLO "[Ee][Hh][Ll][Oo]" RE_SPACE RE_DOMAIN RE_CRLF
#define RE_CMND_MAIL "[Mm][Aa][Ii][Ll] [Ff][Rr][Oo][Mm]:" RE_SPACE RE_EMAIL_OR_EMPTY RE_CRLF
#define RE_CMND_RCPT "[Rr][Cc][Pp][Tt] [Tt][Oo]:" RE_SPACE RE_EMAIL RE_CRLF
#define RE_CMND_VRFY "[Vv][Rr][Ff][Yy]" RE_SPACE RE_DOMAIN RE_CRLF
#define RE_CMND_DATA "[Dd][Aa][Tt][Aa]" RE_CRLF
#define RE_CMND_RSET "[Rr][Ss][Ee][Tt]" RE_CRLF
#define RE_CMND_QUIT "[Qq][Uu][Ii][Tt]" RE_CRLF
/* MAIL CONTENT: */

#define EOL "\r\n"
#define EOM "\r\n.\r\n"

#define EOL_WITH_DOT_MASK "\r\n.."
#define EOL_WITH_DOT "\r\n."


#endif //SMTP_MTA_REXPRS_H
