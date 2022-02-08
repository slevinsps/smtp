#ifndef SMTP_RESPONSES_H
#define SMTP_RESPONSES_H

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
#define TIMEOUT 5 * 60

#endif //SMTP_RESPONSES_H
