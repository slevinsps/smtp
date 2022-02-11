#include <assert.h>
#include <string.h>
#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include "../re_parser.h"

START_TEST(test_helo)
{
    initialize_reg();
    char *debug = (char *)malloc(1000 * sizeof(char));
    const char* text = "helo localhost\r\n";
    int len_text = strlen(text);
    const char* matchdata;
    re_commands command = match_reg(text, &matchdata, len_text);

    re_commands expected_command = SMTP_RE_CMND_HELO;
    char *expected_match = "localhost";
    sprintf(debug, "want %d %s \n got %d %s", expected_command, expected_match, command, matchdata);
    fail_unless((command == expected_command) && (strcmp(expected_match, matchdata) == 0) , debug);
    free(debug);
    if (matchdata)
        free_match_data(matchdata);
    finalize_reg();
}
END_TEST

START_TEST(test_ehlo)
{
    initialize_reg();
    char *debug = (char *)malloc(1000 * sizeof(char));
    const char* text = "ehlo localhost\r\n";
    int len_text = strlen(text);
    const char* matchdata;
    re_commands command = match_reg(text, &matchdata, len_text);

    re_commands expected_command = SMTP_RE_CMND_EHLO;
    char *expected_match = "localhost";
    sprintf(debug, "want %d %s \n got %d %s", expected_command, expected_match, command, matchdata);
    fail_unless((command == expected_command) && (strcmp(expected_match, matchdata) == 0) , debug);
    free(debug);
    if (matchdata)
        free_match_data(matchdata);
    finalize_reg();
}
END_TEST

START_TEST(test_mail_from)
{
    initialize_reg();
    char *debug = (char *)malloc(1000 * sizeof(char));
    const char* text = "mail from: <ivan@mail.ru>\r\n";
    int len_text = strlen(text);
    const char* matchdata;
    re_commands command = match_reg(text, &matchdata, len_text);

    re_commands expected_command = SMTP_RE_CMND_MAIL;
    char *expected_match = "ivan@mail.ru";
    sprintf(debug, "want %d %s \n got %d %s", expected_command, expected_match, command, matchdata);
    fail_unless((command == expected_command) && (strcmp(expected_match, matchdata) == 0) , debug);
    free(debug);
    if (matchdata)
        free_match_data(matchdata);
    finalize_reg();
}
END_TEST

START_TEST(test_rcpt_to)
{
    initialize_reg();
    char *debug = (char *)malloc(1000 * sizeof(char));
    const char* text = "rcpt to: <ivan5@mail.ru>\r\n";
    int len_text = strlen(text);
    const char* matchdata;
    re_commands command = match_reg(text, &matchdata, len_text);

    re_commands expected_command = SMTP_RE_CMND_RCPT;
    char *expected_match = "ivan5@mail.ru";
    sprintf(debug, "want %d %s \n got %d %s", expected_command, expected_match, command, matchdata);
    fail_unless((command == expected_command) && (strcmp(expected_match, matchdata) == 0) , debug);
    free(debug);
    if (matchdata)
        free_match_data(matchdata);
    finalize_reg();
}
END_TEST

START_TEST(test_vrfy)
{
    initialize_reg();
    char *debug = (char *)malloc(1000 * sizeof(char));
    const char* text = "vrfy ivan5@mail.ru\r\n";
    int len_text = strlen(text);
    const char* matchdata;
    re_commands command = match_reg(text, &matchdata, len_text);

    re_commands expected_command = SMTP_RE_CMND_VRFY;
    char *expected_match = "ivan5@mail.ru";
    sprintf(debug, "want %d %s \n got %d %s", expected_command, expected_match, command, matchdata);
    fail_unless((command == expected_command) && (strcmp(expected_match, matchdata) == 0) , debug);
    free(debug);
    if (matchdata)
        free_match_data(matchdata);
    finalize_reg();
}
END_TEST

START_TEST(test_noop)
{
    initialize_reg();
    char *debug = (char *)malloc(1000 * sizeof(char));
    const char* text = "noop\r\n";
    int len_text = strlen(text);
    const char* matchdata = NULL;
    re_commands command = match_reg(text, &matchdata, len_text);

    re_commands expected_command = SMTP_RE_CMND_NOOP;
    sprintf(debug, "want %d %p \n got %d %s", expected_command, NULL, command, matchdata);
    fail_unless((command == expected_command) && (matchdata == NULL) , debug);
    free(debug);
    if (matchdata)
        free_match_data(matchdata);
    finalize_reg();
}
END_TEST

START_TEST(test_quit)
{
    initialize_reg();
    char *debug = (char *)malloc(1000 * sizeof(char));
    const char* text = "quit\r\n";
    int len_text = strlen(text);
    const char* matchdata = NULL;
    re_commands command = match_reg(text, &matchdata, len_text);

    re_commands expected_command = SMTP_RE_CMND_QUIT;
    sprintf(debug, "want %d %p \n got %d %s", expected_command, NULL, command, matchdata);
    fail_unless((command == expected_command) && (matchdata == NULL) , debug);
    free(debug);
    if (matchdata)
        free_match_data(matchdata);
    finalize_reg();
}
END_TEST


START_TEST(test_data)
{
    initialize_reg();
    char *debug = (char *)malloc(1000 * sizeof(char));
    const char* text = "data\r\n";
    int len_text = strlen(text);
    const char* matchdata = NULL;
    re_commands command = match_reg(text, &matchdata, len_text);

    re_commands expected_command = SMTP_RE_CMND_DATA;
    sprintf(debug, "want %d %p \n got %d %s", expected_command, NULL, command, matchdata);
    fail_unless((command == expected_command) && (matchdata == NULL) , debug);
    free(debug);
    if (matchdata)
        free_match_data(matchdata);
    finalize_reg();
}
END_TEST


START_TEST(test_rset)
{
    initialize_reg();
    char *debug = (char *)malloc(1000 * sizeof(char));
    const char* text = "rset\r\n";
    int len_text = strlen(text);
    const char* matchdata = NULL;
    re_commands command = match_reg(text, &matchdata, len_text);

    re_commands expected_command = SMTP_RE_CMND_RSET;
    sprintf(debug, "want %d %p \n got %d %s", expected_command, NULL, command, matchdata);
    fail_unless((command == expected_command) && (matchdata == NULL) , debug);
    free(debug);
    if (matchdata)
        free_match_data(matchdata);
    finalize_reg();
}
END_TEST

START_TEST(test_on_capital)
{
    initialize_reg();
    char *debug = (char *)malloc(1000 * sizeof(char));
    const char* text = "RsEt\r\n";
    int len_text = strlen(text);
    const char* matchdata = NULL;
    re_commands command = match_reg(text, &matchdata, len_text);

    re_commands expected_command = SMTP_RE_CMND_RSET;
    sprintf(debug, "want %d %p \n got %d %s", expected_command, NULL, command, matchdata);
    fail_unless((command == expected_command) && (matchdata == NULL) , debug);
    free(debug);
    if (matchdata)
        free_match_data(matchdata);
    finalize_reg();
}
END_TEST

START_TEST(test_wrong_command)
{
    initialize_reg();
    char *debug = (char *)malloc(1000 * sizeof(char));
    const char* text = "aaa\r\n";
    int len_text = strlen(text);
    const char* matchdata = NULL;
    re_commands command = match_reg(text, &matchdata, len_text);

    re_commands expected_command = (re_commands) SMTP_SERVER_EV_INVALID;
    sprintf(debug, "want %d %p \n got %d %s", expected_command, NULL, command, matchdata);
    fail_unless((command == expected_command) && (matchdata == NULL) , debug);
    free(debug);
    if (matchdata)
        free_match_data(matchdata);
    finalize_reg();
}
END_TEST


int main(void)
{
    Suite *s1 = suite_create("Core");
    TCase *tc1_1 = tcase_create("Core");
    SRunner *sr = srunner_create(s1);
    int nf;

    suite_add_tcase(s1, tc1_1);
    tcase_add_test(tc1_1, test_helo);
    tcase_add_test(tc1_1, test_ehlo);
    tcase_add_test(tc1_1, test_mail_from);
    tcase_add_test(tc1_1, test_rcpt_to);
    tcase_add_test(tc1_1, test_vrfy);
    tcase_add_test(tc1_1, test_noop);
    tcase_add_test(tc1_1, test_quit);
    tcase_add_test(tc1_1, test_data);
    tcase_add_test(tc1_1, test_rset);
    tcase_add_test(tc1_1, test_on_capital);
    tcase_add_test(tc1_1, test_wrong_command);

    srunner_run_all(sr, CK_ENV);
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);

    return nf == 0 ? 0 : 1;
}