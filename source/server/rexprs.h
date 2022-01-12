#ifndef SMTP_MTA_REXPRS_H
#define SMTP_MTA_REXPRS_H

/* COMMON: */

#define RE_CRLF "\\r\\n"
#define RE_SPACE "\\s*"
#define RE_DOMAIN "(?<domain>.+)"
#define RE_EMAIL_OR_EMPTY "(<(?<address>.+@.+)>|<>)"
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

/* RESPONSES: */

#define RE_RESP_READY "220 Service ready\r\n"
#define RE_RESP_CLOSE "221 Service closing transmission channel\r\n"
#define RE_RESP_OK "250 Requested mail action okay, completed\r\n"
#define RE_RESP_START_MAIL "354 Start mail input; end with <CRLF>.<CRLF>\r\n"

#define RE_RESP_ERR_WRONG_CMND "500 Syntax error, command unrecognised\r\n"
#define RE_RESP_ERR_NOT_IMPL_CMND "502 Command not implemented\r\n"
#define RE_RESP_ERR_BAD_SEQ "503 Bad sequence of commands\r\n"
#define RE_RESP_ERR_MAILBOX_UNAVAIL "550 Requested action not taken: mailbox unavailable\r\n"
#define RE_RESP_ERR_MEM_ALLOC "552 Requested mail action aborted: exceeded storage allocation\r\n"
#define RE_RESP_ERR_MAILBOX_INCORRECT "553 Requested action not taken: mailbox name not allowed\r\n"

#endif //SMTP_MTA_REXPRS_H
