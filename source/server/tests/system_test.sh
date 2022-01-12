#!/usr/bin/env bash

((
sleep 2
echo open 127.0.0.1 2048
sleep 4
echo "HELO my.domain"
sleep 1
echo "MAIL FROM: <nastya@gmail.com>"
sleep 1
echo "RCPT TO:   <test@local.ru>"
sleep 1
echo "DATA"
sleep 1
echo "Hi there"
sleep 1
echo "and hi here!:)"
sleep 1
echo "."
sleep 1
echo "QUIT"
sleep 1
) | telnet)