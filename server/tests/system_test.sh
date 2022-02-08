#!/usr/bin/env bash

((
echo open 127.0.0.1 1026
sleep 0.5
echo "HELO my.domain"
sleep 0.5
echo "MAIL FROM: <testsssss@local.ru>"
sleep 0.5
echo "RCPT TO:   <test@local.ru>"
sleep 0.5
echo "RCPT TO:   <test2@local.ru>"
sleep 0.5
echo "RCPT TO: <test3@locafef.ru>"
sleep 0.5
echo "DATA"
sleep 0.5
echo "Hi there"
sleep 0.5
echo "and hi here!:) testsss . Hey ffff."
sleep 0.5
echo ". and hi here!:) testsss привеееет"
sleep 0.1
echo "."
sleep 2
echo "QUIT"
sleep 0.5
) | telnet)