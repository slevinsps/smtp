#!/usr/bin/env bash

((
sleep 0.1
echo open 127.0.0.1 2048
sleep 0.1
echo "HELO my.domain"
sleep 0.1
echo "MAIL FROM: <nastya@gmail.com>"
sleep 0.1
echo "RCPT TO:   <test@local.ru>"
sleep 0.1
echo "DATA"
sleep 0.1
echo "Hi there"
sleep 0.1
echo "and hi here!:) AAAAAAAAAAAAAAAA ."
sleep 0.1
echo "."
sleep 0.1
echo "QUIT"
sleep 0.1
) | telnet)